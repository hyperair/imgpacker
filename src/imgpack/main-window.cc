#include <glibmm/i18n.h>
#include <imgpack/main-window.hh>
#include <imgpack/application.hh>

using ImgPack::MainWindow;
using Gtk::UIManager;

MainWindow::MainWindow (Application &app) :
    app (app),
    uimgr (UIManager::create ())
{
    add (main_vbox);

    init_uimgr ();
    main_vbox.pack_start (*uimgr->get_widget ("/main_menubar"),
                          Gtk::PACK_SHRINK);
    main_vbox.pack_start (*uimgr->get_widget ("/main_toolbar"),
                          Gtk::PACK_SHRINK);

    main_vbox.pack_start (main_pane, Gtk::PACK_EXPAND_WIDGET);
    main_pane.pack1 (image_list, Gtk::SHRINK | Gtk::FILL);
    main_pane.pack2 (preview, Gtk::EXPAND | Gtk::FILL);

    main_vbox.show_all ();

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
        "            <separator />"
        "            <menuitem action=\"QuitAction\" />"
        "        </menu>"
        "        <menu action=\"HelpMenuAction\">"
        "            <menuitem action=\"AboutAction\" />"
        "        </menu>"
        "    </menubar>"
        "    <toolbar name=\"main_toolbar\">"
        "        <toolitem action=\"AddAction\" />"
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
    actions->add (Action::create ("QuitAction", Gtk::Stock::QUIT),
                  sigc::ptr_fun (&Gtk::Main::quit));

    uimgr->insert_action_group (actions);
}

// callbacks
void MainWindow::on_add ()
{
    // TODO: Show add dialog here
}
