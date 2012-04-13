#include <libintl.h>
#include <gtkmm.h>
#include <iostream>
#include <imgpack/application.hh>

#include <config.h>

int main (int argc, char **argv)
{
    // initialize gettext
    bindtextdomain (GETTEXT_PACKAGE, PROGRAMNAME_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    if (!Glib::thread_supported ())
        Glib::thread_init ();

    try {
        ImgPack::Application::create (argc, argv)->run ();

    } catch (std::exception &e) {
        std::cerr << "Uncaught exception in main(). Terminating with exception"
                  << e.what () << std::endl;
        return 1;
    }

    return 0;
}
