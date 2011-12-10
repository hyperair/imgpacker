#include <imgpack/application.hh>

using ImgPack::Application;

Application::Application (int &argc, char **&argv) :
    ::Gtk::Main (argc, argv) {}

void Application::run_impl ()
{
    main_window.show ();
}
