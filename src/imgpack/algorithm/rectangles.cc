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
