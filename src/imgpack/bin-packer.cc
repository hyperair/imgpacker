#include <stdexcept>
#include <cmath>
#include <limits>

#include <nihpp/sharedptrcreator.hh>
#include <autosprintf.h>

#include <imgpack/bin-packer.hh>
#include <imgpack/logger.hh>

namespace ip = ImgPack;

namespace {
    class CompositeRectangle :
        public ip::Rectangle
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


    // Combines two rectangles either horizontally or vertically by equalizing
    // either width or height
    CompositeRectangle::Ptr combine (ip::Rectangle::Ptr rect1,
                                     ip::Rectangle::Ptr rect2,
                                     double target_aspect)
    {
        LOG(info) << "Combining rectangles: "
                  << gnu::autosprintf ("%fx%f",
                                       rect1->width (), rect1->height ())
                  << " and "
                  << gnu::autosprintf ("%fx%f",
                                       rect2->width (), rect2->height ());


        double rect1_aspect = rect1->aspect_ratio ();
        double rect2_aspect = rect2->aspect_ratio ();

        double vertical_ratio =
            rect1_aspect * rect2_aspect / (rect1_aspect + rect2_aspect);

        double horizontal_ratio = rect1_aspect + rect2_aspect;

        double vertical_ratiodist =
            std::abs (target_aspect - vertical_ratio);

        double horizontal_ratiodist =
            std::abs (target_aspect - horizontal_ratio);


        if (vertical_ratiodist < horizontal_ratiodist)
            return VCompositeRectangle::create (rect1, rect2);

        else
            return HCompositeRectangle::create (rect1, rect2);
    }


    class BinPackerImpl :
        public ip::BinPacker,
        public nihpp::SharedPtrCreator<BinPackerImpl>
    {
    public:
        typedef std::shared_ptr<BinPackerImpl> Ptr;
        using nihpp::SharedPtrCreator<BinPackerImpl>::create;

        BinPackerImpl ();
        virtual ~BinPackerImpl () {}

        virtual void target_aspect (double aspect_ratio);
        virtual void source_rectangles (RectangleList rectangles);

        virtual ip::Rectangle::Ptr result ();

    private:
        virtual void run ();

        double aspect_ratio;
        RectangleList rectangles;
    };
}

CompositeRectangle::CompositeRectangle (ip::Rectangle::Ptr rect1,
                                        ip::Rectangle::Ptr rect2) :
    _children {rect1, rect2}
{}


// HCompositeRectangle definitions
HCompositeRectangle::HCompositeRectangle (ip::Rectangle::Ptr rect1,
                                          ip::Rectangle::Ptr rect2) :
    CompositeRectangle (rect1, rect2)
{
    double common_height = std::min (rect1->max_height (),
                                     rect2->max_height ());

    LOG(info) << "Equalizing height to " << common_height;

    rect1->height (common_height);
    rect2->height (common_height);
}

double HCompositeRectangle::width ()
{
    double width = 0;

    for (auto i : _children)
        width += i->width ();

    return width;
}

void HCompositeRectangle::height (double new_height)
{
    LOG(info) << "Scaling composite by height: "
              << height () << " -> " << new_height;

    for (auto i : _children)
        i->height (new_height);
}

void HCompositeRectangle::width (double new_width)
{
    double new_height = new_width / aspect_ratio ();
    height (new_height);
}

double HCompositeRectangle::max_height ()
{
    double retval = std::numeric_limits<double>::max ();

    for (auto i : _children)
        retval = std::min (retval, i->max_height ());

    return retval;
}

double HCompositeRectangle::max_width ()
{
    return aspect_ratio () * max_height ();
}


// VCompositeRectangle definitions
VCompositeRectangle::VCompositeRectangle (ip::Rectangle::Ptr rect1,
                                          ip::Rectangle::Ptr rect2) :
    CompositeRectangle (rect1, rect2)
{
    double common_width = std::min (rect1->max_width (),
                                    rect2->max_width ());

    LOG(info) << "Equalizing width to " << common_width;

    rect1->width (common_width);
    rect2->width (common_width);
}

double VCompositeRectangle::height ()
{
    double height = 0;

    for (auto i : _children)
        height += i->height ();

    return height;
}

void VCompositeRectangle::width (double new_width)
{
    LOG(info) << "Scaling composite by width: "
              << width () << " -> " << new_width;
    for (auto i : _children)
        i->width (new_width);
}

void VCompositeRectangle::height (double new_height)
{
    double new_width = new_height * aspect_ratio ();
    width (new_width);
}

double VCompositeRectangle::max_height ()
{
    return max_width () / aspect_ratio ();
}

double VCompositeRectangle::max_width ()
{
    double retval = std::numeric_limits<double>::max ();

    for (auto i : _children)
        retval = std::min (retval, i->max_width ());

    return retval;
}


// BinPacker definitions
ip::BinPacker::Ptr ip::BinPacker::create ()
{
    return BinPackerImpl::create ();
}

BinPackerImpl::BinPackerImpl () :
    aspect_ratio (210.0 / 297)
{
}

void BinPackerImpl::target_aspect (double aspect_ratio)
{
    this->aspect_ratio = aspect_ratio;
}

void BinPackerImpl::source_rectangles (RectangleList rectangles)
{
    this->rectangles = std::move (rectangles);
}

void BinPackerImpl::run ()
{
    // Hack: Loop until only one rectangle is left
    // This can be replaced with rectangles.size () > 1 with a new gcc
    while (++rectangles.begin () != rectangles.end ()) {
        testcancelled ();

        using ip::Rectangle;
        Rectangle::Ptr rect1 = rectangles.front ();
        rectangles.pop_front ();

        Rectangle::Ptr rect2 = rectangles.front ();
        rectangles.pop_front ();

        Rectangle::Ptr result_rect = combine (rect1, rect2, aspect_ratio);
        rectangles.push_back (result_rect);
    }
}

ip::Rectangle::Ptr BinPackerImpl::result ()
{
    int nrectangles = rectangles.size ();

    if (nrectangles != 1)
        return ip::Rectangle::Ptr ();

    return rectangles.front ();
}
