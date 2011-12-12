#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <gtkmm.h>

namespace ImgPack
{
    class MainWindow : public Gtk::Window
    {
    public:
        MainWindow ();
        ~MainWindow ();

    private:
        Glib::RefPtr<Gtk::UIManager> uimgr;
        void                         init_uimgr ();

        Gtk::VBox                    main_vbox;
        Gtk::HPaned                  main_pane;

        Gtk::IconView                image_list;
        Gtk::DrawingArea             preview;

        // callbacks
        void on_add ();
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
