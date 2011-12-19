#include <unistd.h>
#include <glibmm/i18n.h>

#include <imgpack/application.hh>
#include <imgpack/logger.hh>
#include <config.h>

using ImgPack::Application;

Application::Application (int &argc, char **&argv) :
    ::Gtk::Main (argc, argv),

    main_window (*this),
    thread_pool_ (sysconf (_SC_NPROCESSORS_ONLN))
{
    Glib::set_application_name (_("ImgPacker Collage Creator"));

    // initialize about_dialog
    about_dialog.set_version (VERSION);
    about_dialog.set_license_type (Gtk::LICENSE_GPL_3_0);
    about_dialog.set_authors ({"Chow Loong Jin <hyperair@ubuntu.com>"});
    about_dialog.signal_response ()
        .connect ([&about_dialog](int) {about_dialog.hide ();});

    LOG(info) << "Initialized thread pool with max threads: "
              << thread_pool ().get_max_threads ();
}

void Application::run ()
{
    Gtk::Main::run (main_window);
}

void Application::show_about ()
{
    about_dialog.show ();
}
