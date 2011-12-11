#include <gtkmm.h>
#include <iostream>
#include <imgpack/application.hh>

int main (int argc, char **argv)
{
    try {
        ImgPack::Application (argc, argv).run ();

    } catch (std::exception &e) {
        std::cerr << "Uncaught exception in main(). Terminating with exception"
                  << e.what () << std::endl;
        return 1;
    }

    return 0;
}
