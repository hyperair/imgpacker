#ifndef IMGPACK_ALGORITHMS_RECTANGLES_HH
#define IMGPACK_ALGORITHMS_RECTANGLES_HH

#include <memory>
#include <vector>

#include <nihpp/sharedptrcreator.hh>

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

        class CompositeRectangle :
            public Rectangle
        {
        public:
            typedef std::shared_ptr<CompositeRectangle> Ptr;

            CompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);
            virtual ~CompositeRectangle () {}

            virtual std::vector<Rectangle::Ptr> children () {return _children;}

        protected:
            std::vector<Rectangle::Ptr> _children;
        };

        class HCompositeRectangle :
            public CompositeRectangle,
            public nihpp::SharedPtrCreator<HCompositeRectangle>
        {
        public:
            HCompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);

            virtual double height () {return _children.front ()->height ();}
            virtual double width ();

            virtual void height (double new_height);
            virtual void width (double new_width);

            virtual double max_height ();
            virtual double max_width ();

            virtual Orientation orientation () {return HORIZONTAL;}
        };

        class VCompositeRectangle :
            public CompositeRectangle,
            public nihpp::SharedPtrCreator<VCompositeRectangle>
        {
        public:
            VCompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);

            virtual double height ();
            virtual double width () {return _children.front ()->width ();}

            virtual void height (double new_height);
            virtual void width (double new_width);

            virtual double max_height ();
            virtual double max_width ();

            virtual Orientation orientation () {return VERTICAL;}
        };

    }
}

#endif  // IMGPACK_ALGORITHMS_RECTANGLES_HH
