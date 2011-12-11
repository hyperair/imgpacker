#include <imgpack/application.hh>

using ImgPack::Application;

Application::Application (int &argc, char **&argv) :
    ::Gtk::Main (argc, argv) {}

void Application::run ()
{
    Gtk::Main::run (main_window);
}
