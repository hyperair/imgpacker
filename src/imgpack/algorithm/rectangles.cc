#include <imgpack/algorithm/rectangles.hh>
#include <imgpack/util/logger.hh>
#include <limits>

namespace ip = ImgPack;
namespace ipa = ip::Algorithm;

using ipa::Rectangle;
using ipa::CompositeRectangle;
using ipa::HCompositeRectangle;
using ipa::VCompositeRectangle;

CompositeRectangle::CompositeRectangle (ipa::Rectangle::Ptr rect1,
                                        ipa::Rectangle::Ptr rect2) :
    _children {rect1, rect2}
{}


// HCompositeRectangle definitions
HCompositeRectangle::HCompositeRectangle (ipa::Rectangle::Ptr rect1,
                                          ipa::Rectangle::Ptr rect2) :
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
    return child1 ()->width () + child2 ()->width ();
}

void HCompositeRectangle::height (double new_height)
{
    LOG(info) << "Scaling composite by height: "
              << height () << " -> " << new_height;

    child1 ()->height (new_height);
    child2 ()->height (new_height);
}

void HCompositeRectangle::width (double new_width)
{
    double new_height = new_width / aspect_ratio ();
    height (new_height);
}

double HCompositeRectangle::max_height ()
{
    return std::min (child1 ()->max_height (), child2 ()->max_height ());
}

double HCompositeRectangle::max_width ()
{
    return aspect_ratio () * max_height ();
}


// VCompositeRectangle definitions
VCompositeRectangle::VCompositeRectangle (ipa::Rectangle::Ptr rect1,
                                          ipa::Rectangle::Ptr rect2) :
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
    return child1 ()->height () + child2 ()->height ();
}

void VCompositeRectangle::width (double new_width)
{
    LOG(info) << "Scaling composite by width: "
              << width () << " -> " << new_width;
    child1 ()->width (new_width);
    child2 ()->width (new_width);
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
    return std::min (child1 ()->max_width (), child2 ()->max_width ());
}