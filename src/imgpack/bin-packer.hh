#ifndef IMGPACK_BINPACKER_HH
#define IMGPACK_BINPACKER_HH

#include <memory>
#include <vector>

namespace ImgPack
{
    class Rectangle
    {
    public:
        typedef std::shared_ptr<Rectangle> Ptr;
        enum Orientation {HORIZONTAL, VERTICAL};

        virtual ~Rectangle () {}

        virtual int width () = 0;
        virtual int height () = 0;

        virtual void width (int) = 0;
        virtual void height (int) = 0;

        virtual int max_width () = 0;
        virtual int max_height () = 0;

        // overridden if Rectangle has children
        virtual Orientation orientation () {return HORIZONTAL;}
        virtual std::vector<Ptr> children () {return {};}

        double aspect_ratio () {return width () / height ();}

    protected:
        Rectangle (){}
    };

    class BinPacker
    {
    public:
        typedef std::shared_ptr<BinPacker> Ptr;
        typedef decltype (std::declval<Rectangle> ().children ()) RectangleList;

        static Ptr create ();
        virtual ~BinPacker () {}

        virtual void start () = 0;
        virtual void stop () = 0;

        virtual void target_aspect_ratio (double aspect_ratio) = 0;
        virtual void source_rectangles (const RectangleList &rectangles);

        virtual Rectangle::Ptr result ();

    protected:
        BinPacker (){}
    };
}

#endif  // IMGPACK_BINPACKER_HH
