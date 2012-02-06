#ifndef IMGPACK_BINPACKER_HH
#define IMGPACK_BINPACKER_HH

#include <memory>
#include <vector>

#include <imgpack/async-operation.hh>

namespace ImgPack
{
    namespace Algorithm
    {
        class Rectangle
        {
        public:
            typedef std::shared_ptr<Rectangle> Ptr;
            enum Orientation {INVALID, HORIZONTAL, VERTICAL};

            virtual ~Rectangle () {}

            virtual double width () = 0;
            virtual void width (double) = 0;

            virtual double height () = 0;
            virtual void height (double) = 0;

            virtual double max_width () = 0;
            virtual double max_height () = 0;

            // overridden if Rectangle has children
            virtual Orientation orientation () {return INVALID;}
            virtual std::vector<Ptr> children () {return {};}
            double aspect_ratio () {return width () / height ();}

        protected:
            Rectangle (){}
        };


        class BinPacker : virtual public AsyncOperation
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
}

#endif  // IMGPACK_BINPACKER_HH
