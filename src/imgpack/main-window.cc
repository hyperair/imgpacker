#include <memory>
#include <string>
#include <unordered_set>

#include <glibmm/i18n.h>
#include <imgpack/main-window.hh>
#include <imgpack/application.hh>
#include <imgpack/thread-pool.hh>

using ImgPack::MainWindow;
using ImgPack::ThreadPool;
using Gtk::UIManager;

class MainWindow::PixbufLoader :
    public MainWindow::Operation,
    public nihpp::SharedPtrCreator<PixbufLoader>
{
public:
    using nihpp::SharedPtrCreator<PixbufLoader>::create;
    using nihpp::SharedPtrCreator<PixbufLoader>::Ptr;
    using nihpp::SharedPtrCreator<PixbufLoader>::WPtr;

    PixbufLoader ();
    ~PixbufLoader () {detach ();}

    virtual void attach (MainWindow &mainwindow);
    virtual void detach ();
    bool attached () {return mainwindow;}

    void enqueue (Glib::RefPtr<Gio::File> file);

    std::list<std::tuple<Glib::RefPtr<Gio::File>,
                         Glib::RefPtr<Gdk::Pixbuf> > > &
    get_results () {return results;}

    std::list<std::tuple<Glib::RefPtr<Gio::File>,
                         Glib::ustring> >
    get_errors () {return results;}

private:
    MainWindow *mainwindow;
    guint contextid;

    typedef Glib::RefPtr<Gio::File> file_t;
    typedef Glib::RefPtr<Gdk::Pixbuf> pixbuf_t;
    typedef std::tuple<file_t, pixbuf_t> result_t;

    std::unordered_set<std::string> visited;

    Glib::RefPtr<Gio::Cancellable> cancellable;
    void testcancelled ();

    Glib::Dispatcher progress;
    void on_progress ();

    Glib::Dispatcher thread_finished;

    Glib::Thread *load_pixbufs_thread;
    void load_pixbufs ();

    // helper functions for load_pixbufs()
    file_t get_next_unprocessed ();
    void recurse_file (file_t file);
    void load_pixbuf (file_t file);


    Glib::Mutex mutex;      // Protects everything below this
    std::list<file_t> unprocessed;
    std::list<result_t> results;
    std::list<std::tuple<file_t, Glib::ustring> > errors;
};

// PixbufLoader definitions
MainWindow::PixbufLoader::PixbufLoader () :
    mainwindow (nullptr),
    load_pixbufs_thread (nullptr)
{
    progress.connect
        (sigc::mem_fun (*this, &PixbufLoader::on_progress));
    thread_finished.connect
        (sigc::mem_fun (*this, &PixbufLoader::detach));
}

void MainWindow::PixbufLoader::attach (MainWindow &mainwindow)
{
    g_assert (this->mainwindow == 0);

    this->mainwindow = &mainwindow;

    contextid = mainwindow.statusbar ().get_context_id ("Adding images");
    mainwindow.statusbar ().push (_("Adding images..."),
                                  contextid);
    mainwindow.progressbar ().set_fraction (0.0);

    // Initialize data structures
    g_assert (!cancellable && !load_pixbufs_thread);
    cancellable = Gio::Cancellable::create ();
    results.clear ();
    errors.clear ();

    // Start thread
    load_pixbufs_thread = Glib::Thread::create
        (sigc::mem_fun (*this, &PixbufLoader::load_pixbufs), true);
}

void MainWindow::PixbufLoader::detach ()
{
    if (!attached ())
        return;

    cancellable->cancel ();

    if (load_pixbufs_thread)
        load_pixbufs_thread->join ();

    load_pixbufs_thread = nullptr;
    cancellable.reset ();

    mainwindow = nullptr;

    finish ();
}

void MainWindow::PixbufLoader::enqueue (file_t file)
{
    Glib::Mutex::Lock l (mutex);
    unprocessed.push_back (file);
}

void MainWindow::PixbufLoader::testcancelled ()
{
    if (cancellable->is_cancelled ())
        throw Glib::Thread::Exit ();
}

Glib::RefPtr<Gio::File> MainWindow::PixbufLoader::get_next_unprocessed ()
{
    Glib::Mutex::Lock l (mutex);

    if (!unprocessed.empty ()) {
        file_t file = unprocessed.front ();
        unprocessed.pop_front ();
        return file;

    } else
        return file_t ();
}

namespace {
    struct CleanupHelper
    {
        CleanupHelper (Glib::Dispatcher &dispatcher) :
            dispatcher (dispatcher) {}
        ~CleanupHelper () {dispatcher ();}

        Glib::Dispatcher &dispatcher;
    };
}

// Runs in a separate thread
void MainWindow::PixbufLoader::load_pixbufs ()
{
    CleanupHelper cleanup (thread_finished);

    while (Glib::RefPtr<Gio::File> file = get_next_unprocessed ()) {
        try {
            auto fileinfo = file->query_info (cancellable);
            testcancelled ();

            std::string fileid =
                fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE);

            // Check if visited to avoid recursive loop
            if (visited.find (fileid) != visited.end ())
                    continue;

            switch (fileinfo->get_file_type ()) {
            case Gio::FILE_TYPE_DIRECTORY:
                recurse_file (file);
                break;

            case Gio::FILE_TYPE_REGULAR:
                load_pixbuf (file);
                break;

            default:
                LOG(warning) << "Ignoring file " << file->get_uri ()
                             << " because it has unknown file type "
                             << fileinfo->get_file_type ();

            }

            visited.insert
                (fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE));

        } catch (Gio::Error &e) {
            LOG(warning) << "Skipping file " << file->get_uri ()
                         << "because Gio::Error was thrown: "
                         << e.what ();

        } catch (Glib::Thread::Exit &e) {
            // We were cancelled, so put the file back
            Glib::Mutex::Lock l (mutex);
            unprocessed.push_front (file);

            throw;
        }

        testcancelled ();
    }
}

void MainWindow::PixbufLoader::recurse_file (file_t file)
{
    // Enqueue all children to the back (BFS)
    Glib::RefPtr<Gio::FileEnumerator> fe =
        file->enumerate_children
        (cancellable, "*",
         Gio::FILE_QUERY_INFO_NOFOLLOW_SYMLINKS);

    testcancelled ();

    while (Glib::RefPtr<Gio::FileInfo> i =
           fe->next_file (cancellable)) {
        file_t child = file->resolve_relative_path (i->get_name ());

        enqueue (child);
    }
}

void MainWindow::PixbufLoader::load_pixbuf (Glib::RefPtr<Gio::File> file)
{
    try {
        pixbuf_t pixbuf =
            Gdk::Pixbuf::create_from_stream (file->read ());

        Glib::Mutex::Lock l (mutex);
        result_t result {file, pixbuf};
        results.push_back (result);

        LOG(info) << "Successfully loaded pixbuf from " << file->get_uri ();

    } catch (Glib::Exception &e) {
        typedef decltype (errors) error_list_type;

        typename error_list_type::value_type error {file, e.what ()};
        errors.push_back (error);

        LOG(info) << "Could not load pixbuf from " << file->get_uri () << ": "
                  << e.what ();
    }

    progress ();
}

void MainWindow::PixbufLoader::on_progress ()
{
    Glib::Mutex::Lock l (mutex);

    int results_size = results.size ();
    int errors_size = errors.size ();
    int unprocessed_size = unprocessed.size ();

    int total = results_size + errors_size + unprocessed_size;
    float fraction = float(results_size + errors_size) / total;
    mainwindow->progressbar ().set_fraction (fraction);
}


// MainWindow definitions8
MainWindow::MainWindow (Application &app) :
    app (app),
    uimgr (UIManager::create ()),
    pixbuf_loader (PixbufLoader::create ())
{
    add (main_vbox);

    init_uimgr ();
    // Prepare menubar and toolbar
    main_vbox.pack_start (*uimgr->get_widget ("/main_menubar"),
                          Gtk::PACK_SHRINK);
    main_vbox.pack_start (*uimgr->get_widget ("/main_toolbar"),
                          Gtk::PACK_SHRINK);

    main_vbox.pack_start (main_pane, Gtk::PACK_EXPAND_WIDGET);

    // Prepare main pane
    Gtk::ScrolledWindow *scrolled = Gtk::manage (new Gtk::ScrolledWindow ());
    scrolled->add (image_list);
    scrolled->set_min_content_width (image_list.get_icon_width () + 20);

    main_pane.pack1 (*scrolled, Gtk::SHRINK | Gtk::FILL);
    main_pane.pack2 (preview, Gtk::EXPAND | Gtk::FILL);

    // Prepare statusbar
    statusbar ().pack_end (progressbar (), Gtk::PACK_SHRINK);
    main_vbox.pack_start (statusbar (), Gtk::PACK_SHRINK);

    main_vbox.show_all ();

    // Only show statusbar when operation is active
    statusbar ().hide ();

    set_default_size (640, 480);
}

MainWindow::~MainWindow ()
{
    operation_finish_connection.disconnect ();
}

void
MainWindow::set_operation (const MainWindow::Operation::Ptr &operation)
{
    g_assert (operation);

    if (this->operation)
        throw OperationActive (this->operation);

    this->operation = operation;

    operation_finish_connection = operation->connect_signal_finish ([&] () {
            statusbar ().hide ();
            unset_operation ();
        });

    operation->attach (*this);
    statusbar ().show ();
}

void MainWindow::unset_operation ()
{
    Operation::Ptr operation = this->operation;
    this->operation.reset ();

    if (!operation)
        return;

    operation->detach ();
    operation_finish_connection.disconnect ();
    operation.reset ();
}

void MainWindow::init_uimgr ()
{
    uimgr->add_ui_from_string (
        "<ui>"
        "    <menubar name=\"main_menubar\">"
        "        <menu action=\"FileMenuAction\">"
        "            <menuitem action=\"AddAction\" />"
        "            <menuitem action=\"RemoveAction\" />"
        "            <separator />"
        "            <menuitem action=\"ExecAction\" />"
        "            <separator />"
        "            <menuitem action=\"QuitAction\" />"
        "        </menu>"
        "        <menu action=\"HelpMenuAction\">"
        "            <menuitem action=\"AboutAction\" />"
        "        </menu>"
        "    </menubar>"
        "    <toolbar name=\"main_toolbar\">"
        "        <toolitem action=\"AddAction\" />"
        "        <toolitem action=\"RemoveAction\" />"
        "        <separator />"
        "        <toolitem action=\"ExecAction\" />"
        "    </toolbar>"
        "</ui>"
        );

    Glib::RefPtr<Gtk::ActionGroup> actions = Gtk::ActionGroup::create ();

    using Gtk::Action;
    actions->add (Action::create ("FileMenuAction", _("_File")));
    actions->add (Action::create ("HelpMenuAction", _("_Help")));

    actions->add (Action::create ("AboutAction", _("_About")),
                  sigc::mem_fun (app, &Application::show_about));

    actions->add (Action::create ("AddAction", Gtk::Stock::ADD,
                                  _("Add images")),
                  sigc::mem_fun (*this, &MainWindow::on_add));

    actions->add (Action::create ("RemoveAction", Gtk::Stock::REMOVE,
                                  _("Remove images")),
                  sigc::mem_fun (image_list, &ImageList::remove_selected));

    actions->add (Action::create ("ExecAction", Gtk::Stock::EXECUTE),
                  sigc::mem_fun (*this, &MainWindow::on_exec));

    actions->add (Action::create ("QuitAction", Gtk::Stock::QUIT),
                  sigc::ptr_fun (&Gtk::Main::quit));

    uimgr->insert_action_group (actions);
}

// callbacks
void MainWindow::on_add ()
{
    enum {
        ADD,
        CANCEL
    };

    if (operation && !pixbuf_loader->attached ())
        return;                 // TODO: Show error

    Gtk::FileChooserDialog dialog (*this, _("Select image(s)"));
    dialog.add_button (Gtk::Stock::CANCEL, CANCEL);
    dialog.add_button (Gtk::Stock::OPEN, ADD);

    dialog.set_default_response (ADD);
    dialog.set_select_multiple (true);
    dialog.set_local_only (false);

    // Setup and add filters
    Glib::RefPtr<Gtk::FileFilter> image_filter = Gtk::FileFilter::create ();
    image_filter->add_pixbuf_formats ();
    image_filter->set_name (_("All Images"));
    dialog.add_filter (image_filter);

    Glib::RefPtr<Gtk::FileFilter> all_filter = Gtk::FileFilter::create ();
    all_filter->add_pattern ("*");
    all_filter->set_name (_("All Files"));
    dialog.add_filter (all_filter);

    // Show dialog and process response
    if (dialog.run () == ADD)
        for (Glib::RefPtr<Gio::File> file : dialog.get_files ())
            pixbuf_loader->enqueue (file);

    // If !operation, pixbuf_loader is already the current operation
    if (!operation)
        set_operation (pixbuf_loader);
}

void MainWindow::on_exec ()
{
    // TODO: implement
}
