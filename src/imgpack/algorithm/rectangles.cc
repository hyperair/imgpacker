#include <imgpack/algorithm/rectangles.hh>
#include <imgpack/util/logger.hh>
#include <limits>
#include <glibmm.h>

namespace ip = ImgPack;
namespace ipa = ip::Algorithm;

using ipa::Rectangle;
using ipa::CompositeRectangle;
using ipa::HCompositeRectangle;
using ipa::VCompositeRectangle;

CompositeRectangle::Ptr Rectangle::parent () const
{
    return _parent ? _parent->shared_from_this () : CompositeRectangle::Ptr ();
}

void Rectangle::parent (CompositeRectangle *new_parent)
{
    if (_parent == new_parent)
        return;

    if (_parent)
        _parent->orphan_child (*this);

    _parent = new_parent;
}

void Rectangle::child1 (Ptr)
{
    g_assert_not_reached ();
}

void Rectangle::child2 (Ptr)
{
    g_assert_not_reached ();
}


CompositeRectangle::CompositeRectangle (Rectangle::Ptr rect1,
                                        Rectangle::Ptr rect2) :
    _children {rect1, rect2}
{
    rect1->parent (this);
    rect2->parent (this);
}

Rectangle::Ptr CompositeRectangle::child1 ()
{
    return _children.first;
}

Rectangle::Ptr CompositeRectangle::child2 ()
{
    return _children.second;
}

namespace {
    void set_child (Rectangle::Ptr &dest, Rectangle::Ptr &child,
                    CompositeRectangle &parent)
    {
        if (dest == child)
            return;

        Rectangle::Ptr old_child = std::move (dest);

        old_child->parent (nullptr);
        child->parent (&parent);
        dest = std::move (child);
    }
}

void CompositeRectangle::child1 (Rectangle::Ptr child)
{
    set_child (_children.first, child, *this);
    recalculate_size ();
}

void CompositeRectangle::child2 (Rectangle::Ptr child)
{
    set_child (_children.second, child, *this);
    recalculate_size ();
}

void CompositeRectangle::orphan_child (Rectangle &child)
{
    if (child1 ().get () == &child)
        child1 (Rectangle::Ptr ());

    else if (child2 ().get () == &child)
        child2 (Rectangle::Ptr ());
}

void CompositeRectangle::recalculate_size ()
{
    if (parent ())
        parent ()->recalculate_size ();

    else
        recalculate_size_impl ();
}



// HCompositeRectangle definitions
HCompositeRectangle::HCompositeRectangle (ipa::Rectangle::Ptr rect1,
                                          ipa::Rectangle::Ptr rect2) :
    CompositeRectangle (rect1, rect2)
{
    recalculate_size ();
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

void HCompositeRectangle::recalculate_size_impl ()
{
    double common_height = std::min (child1 ()->max_height (),
                                     child2 ()->max_height ());

    LOG(info) << "Equalizing height to " << common_height;

    child1 ()->height (common_height);
    child2 ()->height (common_height);
}


// VCompositeRectangle definitions
VCompositeRectangle::VCompositeRectangle (ipa::Rectangle::Ptr rect1,
                                          ipa::Rectangle::Ptr rect2) :
    CompositeRectangle (rect1, rect2)
{
    recalculate_size ();
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

void VCompositeRectangle::recalculate_size_impl ()
{
    double common_width = std::min (child1 ()->max_width (),
                                    child2 ()->max_width ());

    LOG(info) << "Equalizing width to " << common_width;

    child1 ()->width (common_width);
    child2 ()->width (common_width);
}
