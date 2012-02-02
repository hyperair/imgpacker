#include <stdexcept>
#include <cmath>
#include <limits>

#include <nihpp/sharedptrcreator.hh>

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

        virtual int height () {return _children.front ()->height ();}
        virtual int width ();

        virtual void height (int new_height);
        virtual void width (int new_width);

        virtual int max_height ();
        virtual int max_width ();

        virtual Orientation orientation () {return HORIZONTAL;}

    private:
        int _max_height;
    };


    class VCompositeRectangle :
        public CompositeRectangle,
        public nihpp::SharedPtrCreator<VCompositeRectangle>
    {
    public:
        VCompositeRectangle (Rectangle::Ptr rect1, Rectangle::Ptr rect2);

        virtual int height ();
        virtual int width () {return _children.front ()->width ();}

        virtual void height (int new_height);
        virtual void width (int new_width);

        virtual int max_height ();
        virtual int max_width ();

        virtual Orientation orientation () {return VERTICAL;}
    };


    // Combines two rectangles either horizontally or vertically by equalizing
    // either width or height
    CompositeRectangle::Ptr combine (ip::Rectangle::Ptr rect1,
                                     ip::Rectangle::Ptr rect2,
                                     double target_aspect)
    {
        LOG(info) << "Combining rectangles of aspect ratios: ["
                  << rect1->aspect_ratio () << "] and ["
                  << rect2->aspect_ratio () << "]";
        CompositeRectangle::Ptr vertical =
            VCompositeRectangle::create (rect1, rect2);

        CompositeRectangle::Ptr horizontal =
            HCompositeRectangle::create (rect1, rect2);

        double vertical_ratiodist =
            std::abs (target_aspect - vertical->aspect_ratio ());

        double horizontal_ratiodist =
            std::abs (target_aspect - horizontal->aspect_ratio ());


        if (vertical_ratiodist < horizontal_ratiodist)
            return vertical;

        else
            return horizontal;
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
    int common_height = std::min (rect1->max_height (),
                                  rect2->max_height ());

    rect1->height (common_height);
    rect2->height (common_height);
}

int HCompositeRectangle::width ()
{
    int width = 0;

    for (auto i : _children)
        width += i->width ();

    return width;
}

void HCompositeRectangle::height (int new_height)
{
    for (auto i : _children)
        i->height (new_height);
}

void HCompositeRectangle::width (int new_width)
{
    int new_height = new_width * aspect_ratio ();
    height (new_height);
}

int HCompositeRectangle::max_height ()
{
    int retval = std::numeric_limits<int>::max ();

    for (auto i : _children)
        retval = std::min (retval, i->max_height ());

    return retval;
}

int HCompositeRectangle::max_width ()
{
    return aspect_ratio () * max_height ();
}


// VCompositeRectangle definitions
VCompositeRectangle::VCompositeRectangle (ip::Rectangle::Ptr rect1,
                                          ip::Rectangle::Ptr rect2) :
    CompositeRectangle (rect1, rect2)
{
    int common_width = std::min (rect1->max_width (),
                                  rect2->max_width ());

    rect1->width (common_width);
    rect2->width (common_width);
}

int VCompositeRectangle::height ()
{
    int height = 0;

    for (auto i : _children)
        height += i->height ();

    return height;
}

void VCompositeRectangle::width (int new_width)
{
    for (auto i : _children)
        i->width (new_width);
}

void VCompositeRectangle::height (int new_height)
{
    int new_width = new_height * aspect_ratio ();
    width (new_width);
}

int VCompositeRectangle::max_height ()
{
    return max_width () / aspect_ratio ();
}

int VCompositeRectangle::max_width ()
{
    int retval = std::numeric_limits<int>::max ();

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
