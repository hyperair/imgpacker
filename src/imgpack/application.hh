#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <gtkmm.h>
#include <imgpack/main-window.hh>

namespace ImgPack
{
    class Application : public ::Gtk::Main
    {
    public:
        Application (int &argc, char **&argv);
        Application (const Application &) = delete;
        virtual ~Application () {}

        void run ();            // Hide Gtk::Main::run

        void show_about ();

    private:
        MainWindow       main_window;
        Gtk::AboutDialog about_dialog;
    };
}

#endif  // IMGPACK_APPLICATION_HH
