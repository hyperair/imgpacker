#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <gtkmm.h>
#include <imgpack/imgpackmainwindow.hh>

namespace ImgPack
{
    class ImgPackApplication : public ::Gtk::Main
    {
    public:
        ImgPackApplication (int &argc, char **&argv);
        virtual ~ImgPackApplication ();

    protected:
        virtual void run_impl ();

    private:
        ImgPackMainWindow main_window;
    };
}

#endif  // IMGPACK_APPLICATION_HH
