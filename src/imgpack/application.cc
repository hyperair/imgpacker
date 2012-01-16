#include <unistd.h>
#include <glibmm/i18n.h>

#include <imgpack/application.hh>
#include <imgpack/logger.hh>
#include <config.h>

using ImgPack::Application;

Application::Application (int &argc, char **&argv) :
    Gtk::Main (argc, argv)
{
    Glib::set_application_name (_("ImgPacker Collage Creator"));

    // initialize about_dialog
    about_dialog.set_version (VERSION);
    about_dialog.set_license_type (Gtk::LICENSE_GPL_3_0);
    about_dialog.set_authors ({"Chow Loong Jin <hyperair@ubuntu.com>"});
    about_dialog.signal_response ()
        .connect ([&about_dialog](int) {about_dialog.hide ();});
}

void Application::run ()
{
    spawn_window ();
    Gtk::Main::run ();
}

void Application::show_about ()
{
    about_dialog.show ();
}

void Application::spawn_window ()
{
    MainWindow::Ptr window = MainWindow::create (*this);
    MainWindow::WPtr weak_window = window;

    windows.insert (window);

    window->signal_hide ().connect ([=, &windows]() {
            LOG(info) << "A window was closed. Removing from window list..";

            windows.erase (weak_window.lock ());

            if (windows.empty ())
                quit ();

            else
                LOG(info) << "Remaining open windows: " << windows.size ();
        });

    window->show ();
}
