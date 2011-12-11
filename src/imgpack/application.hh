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
        virtual ~Application () {}

        void run ();            // Hide Gtk::Main::run

    private:
        MainWindow main_window;
    };
}

#endif  // IMGPACK_APPLICATION_HH
