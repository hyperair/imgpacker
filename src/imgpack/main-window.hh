#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <gtkmm.h>
#include <imgpack/image-list.hh>

namespace ImgPack
{
    class Application;

    class MainWindow : public Gtk::Window
    {
    public:
        explicit MainWindow (Application &app);
        MainWindow (const MainWindow &) = delete;
        ~MainWindow ();

        Gtk::Statusbar &statusbar () {return _statusbar;}
        Gtk::ProgressBar &progressbar () {return _progressbar;}

    private:
        Application                 &app;
        Glib::RefPtr<Gtk::UIManager> uimgr;
        void                         init_uimgr ();

        Gtk::VBox                    main_vbox;
        Gtk::HPaned                  main_pane;

        ImageList                    image_list;
        Gtk::DrawingArea             preview;

        Gtk::Statusbar               _statusbar;
        Gtk::ProgressBar             _progressbar;

        // callbacks
        void on_add ();
        void on_exec ();
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
