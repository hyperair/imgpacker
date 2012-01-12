#include <memory>
#include <glibmm/i18n.h>
#include <imgpack/main-window.hh>
#include <imgpack/application.hh>

using ImgPack::MainWindow;
using ImgPack::StatusClient;
using ImgPack::StatusController;

using Gtk::UIManager;

StatusClient::StatusClient (StatusController &controller) :
    controller (&controller)
{
    controller.statusbar.show ();
}

StatusClient::~StatusClient ()
{
    if (controller)
        controller->statusbar.hide ();
}

void StatusClient::unlink ()
{
    controller = nullptr;
}

Gtk::Statusbar &StatusClient::statusbar ()
{
    return controller->statusbar;
}

Gtk::ProgressBar &StatusClient::progressbar ()
{
    return controller->progressbar;
}


// StatusController definitions
StatusController::StatusController () {}
StatusController::~StatusController ()
{
    StatusClient::Ptr ptr = client.lock ();

    if (ptr)
        ptr->unlink ();
}

StatusClient::Ptr StatusController::request ()
{
    if (!client.expired ())
        throw StatusBusy ();

    StatusClient::Ptr new_client = StatusClient::create (*this);
    client = new_client;
    return new_client;
}


// MainWindow definitions
MainWindow::MainWindow (Application &app) :
    app (app),
    uimgr (UIManager::create ())
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

    prepare_pixbuf_loader ();

    // Show dialog and process response
    if (dialog.run () == ADD)
        for (Glib::RefPtr<Gio::File> file : dialog.get_files ())
            pixbuf_loader->enqueue (file);

    pixbuf_loader->start ();
}

void MainWindow::on_exec ()
{
    // TODO: implement
}

void MainWindow::prepare_pixbuf_loader ()
{
    if (pixbuf_loader)
        return;

    pixbuf_loader = PixbufLoader::create (request_status ());
    pixbuf_loader->connect_signal_finish
        (sigc::mem_fun (*this, &MainWindow::reap_pixbufs));
    pixbuf_loader->connect_signal_abort
        (sigc::mem_fun (*this, &MainWindow::on_pixbuf_abort));
}

void MainWindow::reap_pixbufs ()
{
    auto results = pixbuf_loader->results ();

    for (auto i : results)
        if (*i)
            image_list.add_image (i->file (), i->pixbuf ());

    pixbuf_loader.reset ();
}

void MainWindow::on_pixbuf_abort ()
{
    pixbuf_loader.reset ();
}
