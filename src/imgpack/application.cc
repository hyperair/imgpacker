#include <unistd.h>
#include <glibmm/i18n.h>

#include <imgpack/gtkui/gtk-application.hh>

using ImgPack::Application;

// If another ui is added, this is where we'll differentiate it
Application::Ptr Application::create (int &argc, char **&argv)
{
    return Application::Ptr (new GtkUI::GtkApplication (argc, argv));
}
