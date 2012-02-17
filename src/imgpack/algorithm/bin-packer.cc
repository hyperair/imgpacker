#include <stdexcept>
#include <cmath>
#include <limits>

#include <nihpp/sharedptrcreator.hh>
#include <autosprintf.h>

#include <imgpack/algorithm/bin-packer.hh>
#include <imgpack/util/logger.hh>

namespace ip = ImgPack;
namespace ipa = ip::Algorithm;

namespace {
    double le1_ratio (const double v1, const double v2)
    {
        return (v1 < v2) ? v1 / v2 : v2 / v1;
    }

    // Combines two rectangles either horizontally or vertically by equalizing
    // either width or height
    ipa::CompositeRectangle::Ptr combine (ipa::Rectangle::Ptr rect1,
                                          ipa::Rectangle::Ptr rect2,
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

        double vertical_ratio_ratio = le1_ratio (target_aspect,
                                                 vertical_ratio);

        double horizontal_ratio_ratio = le1_ratio (target_aspect,
                                                   horizontal_ratio);

        g_assert (vertical_ratio_ratio <= 1);
        g_assert (horizontal_ratio_ratio <= 1);

        if (vertical_ratio_ratio > horizontal_ratio_ratio)
            return ipa::VCompositeRectangle::create (rect1, rect2);

        else
            return ipa::HCompositeRectangle::create (rect1, rect2);
    }


    class BinPackerImpl :
        public ipa::BinPacker,
        public nihpp::SharedPtrCreator<BinPackerImpl>
    {
    public:
        typedef std::shared_ptr<BinPackerImpl> Ptr;
        using nihpp::SharedPtrCreator<BinPackerImpl>::create;

        BinPackerImpl ();
        virtual ~BinPackerImpl () {}

        virtual void target_aspect (double aspect_ratio);
        virtual void source_rectangles (RectangleList rectangles);

        virtual ipa::Rectangle::Ptr result ();

    private:
        virtual void run ();

        double aspect_ratio;
        RectangleList rectangles;
    };
}


// BinPacker definitions
ipa::BinPacker::Ptr ipa::BinPacker::create ()
{
    return BinPackerImpl::create ();
}

BinPackerImpl::BinPackerImpl () :
    AsyncOperation ("BinPacker"),
    aspect_ratio (1)
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

        using ipa::Rectangle;
        Rectangle::Ptr rect1 = rectangles.front ();
        rectangles.pop_front ();

        Rectangle::Ptr rect2 = rectangles.front ();
        rectangles.pop_front ();

        Rectangle::Ptr result_rect = combine (rect1, rect2, aspect_ratio);
        rectangles.push_back (result_rect);
    }
}

ipa::Rectangle::Ptr BinPackerImpl::result ()
{
    int nrectangles = rectangles.size ();

    if (nrectangles != 1)
        return ipa::Rectangle::Ptr ();

    return rectangles.front ();
}
