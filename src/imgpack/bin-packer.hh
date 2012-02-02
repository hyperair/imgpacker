#ifndef IMGPACK_BINPACKER_HH
#define IMGPACK_BINPACKER_HH

#include <memory>
#include <vector>

#include <imgpack/async-operation.hh>

namespace ImgPack
{
    class Rectangle
    {
    public:
        typedef std::shared_ptr<Rectangle> Ptr;
        enum Orientation {INVALID, HORIZONTAL, VERTICAL};

        virtual ~Rectangle () {}

        virtual int width () = 0;
        virtual void width (int) = 0;

        virtual int height () = 0;
        virtual void height (int) = 0;

        virtual int max_width () = 0;
        virtual int max_height () = 0;

        // overridden if Rectangle has children
        virtual Orientation orientation () {return INVALID;}
        virtual std::vector<Ptr> children () {return {};}
        double aspect_ratio () {return double (width ()) / height ();}

    protected:
        Rectangle (){}
    };


    class BinPacker : public AsyncOperation
    {
    public:
        typedef std::shared_ptr<BinPacker> Ptr;
        typedef std::list<Rectangle::Ptr> RectangleList;

        static Ptr create ();
        virtual ~BinPacker () {}

        virtual void target_aspect (double aspect_ratio) = 0;
        virtual void source_rectangles (RectangleList rectangles) = 0;

        virtual Rectangle::Ptr result () = 0;

    protected:
        BinPacker () {}
    };
}

#endif  // IMGPACK_BINPACKER_HH
