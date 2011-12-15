#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <gtkmm.h>
#include <imgpack/image-loader.hh>

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

        ImageLoader                  image_loader;

        // callbacks
        void on_add ();
        void on_add_finish (Glib::RefPtr<Gio::File> file,
                            Glib::RefPtr<Gdk::Pixbuf> pixbuf);
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
