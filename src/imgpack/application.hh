#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <memory>

namespace ImgPack
{
    class Application
    {
    public:
        typedef std::shared_ptr<Application> Ptr;
        static Ptr create (int &argc, char **&argv);

        virtual ~Application () {}

        virtual void run ()          = 0;
        virtual void show_about ()   = 0;
        virtual void spawn_window () = 0;

    protected:
        Application () {}
        Application (const Application &) = delete;
    };
}

#endif  // IMGPACK_APPLICATION_HH
