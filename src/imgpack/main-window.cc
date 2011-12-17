#include <memory>
#include <glibmm/i18n.h>
#include <imgpack/main-window.hh>
#include <imgpack/application.hh>

using ImgPack::MainWindow;
using Gtk::UIManager;

MainWindow::MainWindow (Application &app) :
    app (app),
    uimgr (UIManager::create ()),

    image_list (app)
{
    add (main_vbox);

    init_uimgr ();
    main_vbox.pack_start (*uimgr->get_widget ("/main_menubar"),
                          Gtk::PACK_SHRINK);
    main_vbox.pack_start (*uimgr->get_widget ("/main_toolbar"),
                          Gtk::PACK_SHRINK);

    main_vbox.pack_start (main_pane, Gtk::PACK_EXPAND_WIDGET);

    Gtk::ScrolledWindow *scrolled = Gtk::manage (new Gtk::ScrolledWindow ());
    scrolled->add (image_list);
    main_pane.pack1 (*scrolled, Gtk::SHRINK | Gtk::FILL);
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
        "            <menuitem action=\"RemoveAction\" />"
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
            image_list.add_image_async (file);
}
