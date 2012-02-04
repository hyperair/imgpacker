#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <unordered_set>
#include <memory>
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

        void spawn_window ();

    private:
        std::unordered_set<std::shared_ptr<MainWindow> >  windows;
        Gtk::AboutDialog about_dialog;
    };
}

#endif  // IMGPACK_APPLICATION_HH
