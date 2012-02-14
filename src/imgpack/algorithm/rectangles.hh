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
            enum Orientation {NONE, HORIZONTAL, VERTICAL};

            virtual ~Rectangle () {}

            virtual double width () = 0;
            virtual void width (double) = 0;

            virtual double height () = 0;
            virtual void height (double) = 0;

            virtual double max_width () = 0;
            virtual double max_height () = 0;

            // overridden if Rectangle has children
            virtual Orientation orientation () {return NONE;}
            virtual Ptr child1 () {return Ptr ();}
            virtual Ptr child2 () {return Ptr ();}

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

            virtual Rectangle::Ptr child1 () {return _children.first;}
            virtual Rectangle::Ptr child2 () {return _children.second;}

        protected:
            std::pair<Rectangle::Ptr, Rectangle::Ptr> _children;
        };

        class HCompositeRectangle :
            public CompositeRectangle,
            public nihpp::SharedPtrCreator<HCompositeRectangle>
        {
        public:
            HCompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);

            virtual double height () {return child1 ()->height ();}
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
            virtual double width () {return child1 ()->width ();}

            virtual void height (double new_height);
            virtual void width (double new_width);

            virtual double max_height ();
            virtual double max_width ();

            virtual Orientation orientation () {return VERTICAL;}
        };

    }
}

#endif  // IMGPACK_ALGORITHMS_RECTANGLES_HH
