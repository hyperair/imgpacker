#ifndef _IMGPACK_COLLAGE_GENERATOR_HH
#define _IMGPACK_COLLAGE_GENERATOR_HH

#include <memory>
#include <list>

#include <gtkmm.h>

namespace ImgPack
{
    class Rectangle;


    struct RectangleChild
    {
        int offset_x;
        int offset_y;

        std::shared_ptr<Rectangle> rectangle;
    };

    class Rectangle
    {
    public:
        typedef std::shared_ptr<Rectangle> Ptr;
        typedef std::weak_ptr<Rectangle> WPtr;

        virtual double aspect_ratio () = 0;
        virtual int height () = 0;
        virtual int width () = 0;

        virtual std::list<RectangleChild> children () = 0;

        virtual ~Rectangle () {}
    };


    class CollageGenerator
    {
    public:
        typedef std::shared_ptr<CollageGenerator> Ptr;
        typedef std::weak_ptr<CollageGenerator> WPtr;

        virtual ~CollageGenerator () {}

        virtual void add_source (const Glib::RefPtr<Gdk::Pixbuf> &pixbuf) = 0;
        virtual void start () = 0;
        virtual void abort () = 0;

        virtual void connect_signal_finish (sigc::slot<void> slot) = 0;
        virtual void connect_signal_abort (sigc::slot<void> slot) = 0;

        virtual Rectangle::Ptr result () = 0;

    protected:
        CollageGenerator () {}
        CollageGenerator (const CollageGenerator &) = delete;
    };
}

#endif  // _IMGPACK_COLLAGE_GENERATOR_HH
