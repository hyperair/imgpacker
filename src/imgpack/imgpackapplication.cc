#include <imgpack/imgpackapplication.hh>

using ImgPack::imgpackerApplication;

ImgPackApplication::ImgPackApplication (int &argc, char **&argv) :
    ::Gtk::Main (argc, argv) {}

ImgPackApplication::~ImgPackApplication () {}

void ImgPackApplication::run_impl ()
{
    main_window.show ();
}
