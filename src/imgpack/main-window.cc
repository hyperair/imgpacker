#include <imgpack/main-window.hh>

using ImgPack::MainWindow;
using Gtk::UIManager;

namespace
{
    void init_uimgr (Glib::RefPtr<Gtk::UIManager> &uimgr)
    {
        uimgr->add_ui_from_string (
            "<ui>"
            "    <menubar name=\"main_menubar\">"
            "        <menu name=\"file_menu\" action=\"FileMenuAction\">"
            "            <menuitem name=\"quit\" action=\"QuitAction\" />"
            "        </menu>"
            "        <menu name=\"help_menu\" action=\"HelpMenuAction\">"
            "            <menuitem name=\"about\" action=\"AboutAction\" />"
            "        </menu>"
            "    </menubar>"
            "    <toolbar name=\"main_toolbar\">"
            "    </toolbar>"
            "</ui>"
            );

        Glib::RefPtr<Gtk::ActionGroup> actions = Gtk::ActionGroup::create ();

        using Gtk::Action;
        actions->add (Action::create ("FileMenuAction", "_File"));
        actions->add (Action::create ("QuitAction", Gtk::Stock::QUIT),
                      sigc::ptr_fun (&Gtk::Main::quit));
        actions->add (Action::create ("HelpMenuAction", "_Help"));
        actions->add (Action::create ("AboutAction", "_About"));

        uimgr->insert_action_group (actions);
    }
}

MainWindow::MainWindow () :
    uimgr (UIManager::create ())
{
    add (main_vbox);

    init_uimgr (uimgr);
    main_vbox.add (*uimgr->get_widget ("/main_menubar"));
    main_vbox.add (*uimgr->get_widget ("/main_toolbar"));

    main_vbox.add (main_pane);
    main_pane.add (image_list);
    main_pane.add (preview);

    main_vbox.show_all ();
}

MainWindow::~MainWindow ()
{
}
