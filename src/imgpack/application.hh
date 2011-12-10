#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <gtkmm.h>
#include <imgpack/mainwindow.hh>

namespace ImgPack
{
    class Application : public ::Gtk::Main
    {
    public:
        Application (int &argc, char **&argv);
        virtual ~Application () {}

    protected:
        virtual void run_impl (); // inherited from Gtk::Main

    private:
        MainWindow main_window;
    };
}

#endif  // IMGPACK_APPLICATION_HH
