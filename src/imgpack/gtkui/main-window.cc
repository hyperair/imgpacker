#include <memory>
#include <glibmm/i18n.h>
#include <nihpp/singleton.hh>
#include <imgpack/gtkui/main-window.hh>
#include <imgpack/application.hh>
#include <imgpack/gtkui/image-list.hh>
#include <imgpack/gtkui/pixbuf-loader.hh>
#include <imgpack/util/logger.hh>
#include <imgpack/gtkui/collage-viewer.hh>
#include <imgpack/gtkui/collage-treeview.hh>

namespace ip = ImgPack;
namespace ipg = ip::GtkUI;

class ipg::StatusController
{
public:
    StatusController () {}
    StatusController (const StatusController &) = delete;
    ~StatusController ();

    StatusClient::Ptr request ();

    Gtk::Statusbar      statusbar;
    Gtk::ProgressBar    progressbar;

private:
    StatusClient::WPtr client;
};


ipg::StatusClient::StatusClient (StatusController &controller) :
    controller (&controller)
{
    controller.statusbar.show ();
}

ipg::StatusClient::~StatusClient ()
{
    if (controller)
        controller->statusbar.hide ();
}

Gtk::Statusbar &ipg::StatusClient::statusbar ()
{
    g_assert (controller);
    return controller->statusbar;
}

Gtk::ProgressBar &ipg::StatusClient::progressbar ()
{
    g_assert (controller);
    return controller->progressbar;
}


// StatusController definitions
ipg::StatusController::~StatusController ()
{
    StatusClient::Ptr ptr = client.lock ();

    if (ptr)
        ptr->unlink ();
}

ipg::StatusClient::Ptr ipg::StatusController::request ()
{
    if (!client.expired ())
        throw StatusBusy ();

    StatusClient::Ptr new_client (new StatusClient (*this));
    client = new_client;
    return new_client;
}


// Helper classes and functions
namespace {
    class SelectableCellRendererText : public Gtk::CellRendererText
    {
    public:
        SelectableCellRendererText ()
        {
            property_mode () =  Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
        }

    protected:
        virtual void on_editing_started (Gtk::CellEditable *editable,
                                         const Glib::ustring &)
        {
            Gtk::Entry *entry = dynamic_cast<Gtk::Entry *> (editable);

            if (entry)
                entry->property_editable () = false;
        }
    };


    class ImportErrorDialog : public Gtk::MessageDialog
    {
        struct Columns;
        bool _has_errors;
        Glib::RefPtr<Gtk::ListStore> model;

    public:
        ImportErrorDialog (Gtk::Window &parent);

        void add_error (const Glib::RefPtr<Gio::File> &file,
                        const Glib::ustring &message);

        bool has_errors () {return _has_errors;}
    };


    struct ImportErrorDialog::Columns :
        public Gtk::TreeModel::ColumnRecord,
        public nihpp::Singleton<Columns>
    {
        Gtk::TreeModelColumn<std::string> uri;
        Gtk::TreeModelColumn<Glib::ustring> message;

        Columns () {add (uri); add (message);}
    };

    Gtk::TreeViewColumn &
    make_selectable_text_column (const Glib::ustring &title,
                 const Gtk::TreeModelColumnBase &model_column)
    {
        Gtk::TreeViewColumn *column = manage (new Gtk::TreeViewColumn);
        Gtk::CellRendererText *renderer =
            manage (new SelectableCellRendererText);

        renderer->property_editable () = true;

        column->pack_start (*renderer, Gtk::PACK_EXPAND_WIDGET);
        column->set_renderer (*renderer, model_column);
        column->set_resizable ();
        column->set_title (title);

        return *column;
    }

    struct ImageChooserDialog : Gtk::FileChooserDialog
    {
    public:
        enum Response {
            OK,
            CANCEL
        };

        enum Role {
            OPEN,
            SAVE
        };

        ImageChooserDialog (Gtk::Window &parent, Role role);
    };
}

ImportErrorDialog::ImportErrorDialog (Gtk::Window &parent) :
    Gtk::MessageDialog (parent, _("Import Error"), false,
                        Gtk::MESSAGE_ERROR),

    _has_errors (false),
    model (Gtk::ListStore::create (Columns::instance ()))
{
    set_secondary_text (_("Some files could not be imported."));
    set_resizable (true);
    set_title (_("Import Error"));

    Gtk::TreeView *view = manage (new Gtk::TreeView (model));
    view->append_column
        (make_selectable_text_column (_("URI"), Columns::instance ().uri));
    view->append_column
        (make_selectable_text_column (_("Error message"),
                                      Columns::instance ().message));

    // Add things to vbox
    Gtk::Box *vbox = get_content_area ();
    Gtk::ScrolledWindow *scrolled = manage (new Gtk::ScrolledWindow ());
    scrolled->set_min_content_width (400);
    scrolled->set_min_content_height (200);
    scrolled->add (*view);

    vbox->pack_start (*scrolled, Gtk::PACK_EXPAND_WIDGET);

    scrolled->show_all ();

    // Set default size to prop window open
    set_default_size (600, -1);
}

void ImportErrorDialog::add_error (const Glib::RefPtr<Gio::File> &file,
                             const Glib::ustring &message)
{
    auto iter = model->append ();
    iter->set_value (Columns::instance ().uri, file->get_uri ());
    iter->set_value (Columns::instance ().message, message);

    _has_errors = true;
}

ImageChooserDialog::ImageChooserDialog (Gtk::Window &parent, Role role) :
    Gtk::FileChooserDialog (parent, _("Select image(s)"),
                            role == OPEN ? Gtk::FILE_CHOOSER_ACTION_OPEN :
                                           Gtk::FILE_CHOOSER_ACTION_SAVE)
{
    add_button (Gtk::Stock::CANCEL, CANCEL);
    add_button (role == OPEN ? Gtk::Stock::OPEN : Gtk::Stock::SAVE, OK);

    set_default_response (OK);
    set_select_multiple (true);
    set_local_only (false);

    // Set up image filter
    Glib::RefPtr<Gtk::FileFilter> image_filter = Gtk::FileFilter::create ();
    image_filter->add_pixbuf_formats ();
    image_filter->set_name (_("All Images"));

    // Set up all files filter
    Glib::RefPtr<Gtk::FileFilter> all_filter = Gtk::FileFilter::create ();
    all_filter->add_pattern ("*");
    all_filter->set_name (_("All Files"));

    // Add filters
    add_filter (image_filter);
    add_filter (all_filter);
}


// MainWindow definitions
struct ipg::MainWindow::Private : sigc::trackable
{
    Private (Application &app, MainWindow &self);

    Application                  &app;
    MainWindow                   &self;
    Glib::RefPtr<Gtk::UIManager>  uimgr;

    void                          init_uimgr ();

    Gtk::VBox                     main_vbox;
    Gtk::HPaned                   main_pane;
    Gtk::Notebook                 sidebar_notebook;

    ImageList                     image_list;
    CollageViewer                 viewer;
    CollageTreeView               layout_editor;

    StatusController              status;
    Gtk::Statusbar &statusbar ()        {return status.statusbar;}
    Gtk::ProgressBar &progressbar ()    {return status.progressbar;}

    PixbufLoader::Ptr             pixbuf_loader;

    void                          on_add_clicked ();
    void                          on_export ();
    void                          on_exec ();
    void                          on_new_window ();

    void                          prepare_pixbuf_loader ();
    void                          reap_pixbufs ();
    void                          on_pixbuf_abort ();
};

ipg::MainWindow::Private::Private (Application &app, MainWindow &self) :
    app (app),
    self (self),
    uimgr (Gtk::UIManager::create ())
{
    init_uimgr ();

    // Prepare menubar and toolbar
    main_vbox.pack_start (*uimgr->get_widget ("/main_menubar"),
                          Gtk::PACK_SHRINK);
    main_vbox.pack_start (*uimgr->get_widget ("/main_toolbar"),
                          Gtk::PACK_SHRINK);

    main_vbox.pack_start (main_pane, Gtk::PACK_EXPAND_WIDGET);

    // Prepare main pane
    Gtk::ScrolledWindow *scrolled1 = new Gtk::ScrolledWindow ();
    scrolled1->add (image_list);
    scrolled1->set_min_content_width (image_list.get_icon_width () + 30);

    Gtk::ScrolledWindow *scrolled2 = new Gtk::ScrolledWindow ();
    scrolled2->add (layout_editor);

    sidebar_notebook.append_page (*manage (scrolled1), _("Image List"));
    sidebar_notebook.append_page (*manage (scrolled2), _("Layout Editor"));
    main_pane.pack1 (sidebar_notebook, Gtk::SHRINK | Gtk::FILL);

    Gtk::ScrolledWindow *scrolled3 = new Gtk::ScrolledWindow ();
    scrolled3->add (viewer);
    main_pane.pack2 (*manage (scrolled3), Gtk::EXPAND | Gtk::FILL);

    // Prepare statusbar
    statusbar ().pack_end (progressbar (), Gtk::PACK_SHRINK);
    main_vbox.pack_start (statusbar (), Gtk::PACK_SHRINK);

    main_vbox.show_all ();

    // Connect collage viewer and layout editor
    viewer.connect_signal_update (sigc::mem_fun (layout_editor,
                                                 &CollageTreeView::collage));

    // Only show statusbar when operation is active
    statusbar ().hide ();
}

ipg::MainWindow::MainWindow (Application &app) :
    _priv (new Private (app, *this))
{
    add (_priv->main_vbox);
    add_accel_group (_priv->uimgr->get_accel_group ());
    set_default_size (640, 480);
}

ipg::MainWindow::~MainWindow () {} // For unique_ptr

ipg::StatusClient::Ptr ipg::MainWindow::request_status ()
{
    return _priv->status.request ();
}

void ipg::MainWindow::Private::init_uimgr ()
{
    uimgr->add_ui_from_string (
        "<ui>"
        "    <menubar name=\"main_menubar\">"
        "        <menu action=\"FileMenuAction\">"
        "            <menuitem action=\"NewAction\" />"
        "            <separator />"
        "            <menuitem action=\"AddAction\" />"
        "            <menuitem action=\"RemoveAction\" />"
        "            <separator />"
        "            <menuitem action=\"ExportAction\" />"
        "            <separator />"
        "            <menuitem action=\"ExecAction\" />"
        "            <separator />"
        "            <menuitem action=\"CloseAction\" />"
        "            <menuitem action=\"QuitAction\" />"
        "        </menu>"
        "        <menu action=\"HelpMenuAction\">"
        "            <menuitem action=\"AboutAction\" />"
        "        </menu>"
        "    </menubar>"
        "    <toolbar name=\"main_toolbar\">"
        "        <toolitem action=\"NewAction\" />"
        "        <separator />"
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

    actions->add (Action::create ("NewAction", Gtk::Stock::NEW,
                                  _("New Collage")),
                  sigc::mem_fun (*this, &Private::on_new_window));

    actions->add (Action::create ("AddAction", Gtk::Stock::ADD,
                                  _("Add images")),
                  Gtk::AccelKey ("<Alt>A"),
                  sigc::mem_fun (*this, &Private::on_add_clicked));

    actions->add (Action::create ("RemoveAction", Gtk::Stock::REMOVE,
                                  _("Remove images")),
                  sigc::mem_fun (image_list,
                                 &ImageList::remove_selected));

    actions->add (Action::create ("ExportAction", Gtk::Stock::SAVE),
                  sigc::mem_fun (*this, &Private::on_export));

    actions->add (Action::create ("ExecAction", Gtk::Stock::EXECUTE),
                  sigc::mem_fun (*this, &Private::on_exec));

    actions->add (Action::create ("CloseAction", Gtk::Stock::CLOSE),
                  sigc::mem_fun (self, &MainWindow::hide));

    actions->add (Action::create ("QuitAction", Gtk::Stock::QUIT),
                  sigc::ptr_fun (&Gtk::Main::quit));

    uimgr->insert_action_group (actions);
}

// callbacks
void ipg::MainWindow::Private::on_add_clicked ()
{
    ImageChooserDialog dialog (self, ImageChooserDialog::OPEN);

    prepare_pixbuf_loader ();

    // Show dialog and process response
    if (dialog.run () == ImageChooserDialog::OK)
        for (Glib::RefPtr<Gio::File> file : dialog.get_files ())
            pixbuf_loader->enqueue (file);

    pixbuf_loader->start ();
}

void ipg::MainWindow::Private::on_export ()
{
    ImageChooserDialog dialog (self, ImageChooserDialog::SAVE);

    if (dialog.run () == ImageChooserDialog::OK) {
        Glib::RefPtr<Gio::File> file = dialog.get_file ();

        std::string filename = file->get_basename ();
        size_t offset = filename.find_last_of ('.');

        if (offset == std::string::npos || offset == filename.length () - 1)
            return;             // TODO: Alert user and try again

        std::string extension = filename.substr (++offset);

        auto formats = Gdk::Pixbuf::get_formats ();

        for (const auto &i : formats) {
            if (i.get_name () == extension) {
                viewer.export_to_file (file, i);
                return;
            }
        }

        // TODO: Alert user and try again.
    }
}

void ipg::MainWindow::Private::on_exec ()
{
    viewer.set_source_pixbufs (image_list.pixbufs ());
}

void ipg::MainWindow::Private::on_new_window ()
{
    app.spawn_window ();
}

void ipg::MainWindow::Private::prepare_pixbuf_loader ()
{
    if (pixbuf_loader)
        return;

    pixbuf_loader = PixbufLoader::create (self.request_status ());
    pixbuf_loader->connect_signal_finish
        (sigc::mem_fun (*this, &Private::reap_pixbufs));
    pixbuf_loader->connect_signal_abort
        (sigc::mem_fun (*this, &Private::on_pixbuf_abort));
}


void ipg::MainWindow::Private::reap_pixbufs ()
{
    auto results = pixbuf_loader->results ();

    ImportErrorDialog errors (self);

    for (auto i : results)
        if (*i)
            image_list.add_image (i->file (), i->pixbuf ());

        else
            errors.add_error (i->file (), i->message ());

    pixbuf_loader.reset ();

    if (errors.has_errors ())
        errors.run ();
}

void ipg::MainWindow::Private::on_pixbuf_abort ()
{
    pixbuf_loader.reset ();
}
