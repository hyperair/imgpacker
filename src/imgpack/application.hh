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

        void show_about ();

        Glib::ThreadPool &thread_pool () {return thread_pool_;}

    private:
        MainWindow       main_window;
        Gtk::AboutDialog about_dialog;
        Glib::ThreadPool thread_pool_;
    };
}

#endif  // IMGPACK_APPLICATION_HH
