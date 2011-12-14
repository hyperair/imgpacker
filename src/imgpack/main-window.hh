#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <gtkmm.h>

namespace ImgPack
{
    class Application;

    class MainWindow : public Gtk::Window
    {
    public:
        MainWindow (Application &app);
        ~MainWindow ();

    private:
        Application                 &app;
        Glib::RefPtr<Gtk::UIManager> uimgr;
        void                         init_uimgr ();

        Gtk::VBox                    main_vbox;
        Gtk::HPaned                  main_pane;

        Glib::RefPtr<Gtk::ListStore> image_list_model;
        Gtk::IconView                image_list_view;
        Gtk::DrawingArea             preview;

        // callbacks
        void on_add ();
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
