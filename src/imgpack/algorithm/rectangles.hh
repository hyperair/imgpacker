#ifndef IMGPACK_ALGORITHMS_RECTANGLES_HH
#define IMGPACK_ALGORITHMS_RECTANGLES_HH

#include <memory>
#include <vector>

#include <nihpp/sharedptrcreator.hh>

namespace ImgPack
{
    namespace Algorithm
    {
        class CompositeRectangle;

        class Rectangle :
            public std::enable_shared_from_this<Rectangle>
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

            virtual void child1 (Ptr child);
            virtual void child2 (Ptr child);

            std::shared_ptr<CompositeRectangle> parent () const;
            void parent (CompositeRectangle *new_parent);

            double aspect_ratio () {return width () / height ();}

        protected:
            Rectangle () : _parent (nullptr) {}

        private:
            CompositeRectangle *_parent;
        };


        class CompositeRectangle :
            public Rectangle
        {
        public:
            typedef std::shared_ptr<CompositeRectangle> Ptr;

            CompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);
            virtual ~CompositeRectangle () {}

            virtual Rectangle::Ptr child1 ();
            virtual Rectangle::Ptr child2 ();

            virtual void child1 (Rectangle::Ptr child);
            virtual void child2 (Rectangle::Ptr child);

            void orphan_child (Rectangle &child);

            void recalculate_size ();

            std::shared_ptr<const CompositeRectangle> shared_from_this () const;
            Ptr shared_from_this ();

        protected:
            virtual void recalculate_size_impl () = 0;
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

        private:
            virtual void recalculate_size_impl ();
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

        private:
            virtual void recalculate_size_impl ();
        };
    }
}

#endif  // IMGPACK_ALGORITHMS_RECTANGLES_HH
