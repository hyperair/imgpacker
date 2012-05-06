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

    // Move _parent away in case of recursion
    CompositeRectangle* tmp = _parent;
    _parent = nullptr;

    if (tmp)
        tmp->orphan_child (*this);

    g_assert (!tmp || (tmp->child1 ().get () != this &&
                       tmp->child2 ().get () != this));

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

double Rectangle::offset_x ()
{
    if (!_parent)
        return 0;

    double parent_x = _parent->offset_x ();

    if (_parent->orientation () == VERTICAL)
        return parent_x;

    auto child1 = _parent->child1 ();
    if (child1.get () == this)
        return parent_x;

    return child1->width () + parent_x;
}

double Rectangle::offset_y ()
{
    if (!_parent)
        return 0;

    double parent_y = _parent->offset_y ();

    if (_parent->orientation () == HORIZONTAL)
        return parent_y;

    auto child1 = _parent->child1 ();
    if (child1.get () == this)
        return parent_y;

    return child1->height () + parent_y;
}

Rectangle::Ptr Rectangle::find_rect (double x, double y)
{
    if (x <= width () && y <= height ())
        return shared_from_this ();

    else
        return Rectangle::Ptr ();
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

        if (old_child)
            old_child->parent (nullptr);

        if (child)
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
    if (_children.first.get () == &child)
        _children.first.reset ();

    else if (child2 ().get () == &child)
        _children.second.reset ();
}

void CompositeRectangle::recalculate_size ()
{
    recalculate_size_impl ();

    if (parent ())
        parent ()->recalculate_size ();
}

std::shared_ptr<const CompositeRectangle>
CompositeRectangle::shared_from_this () const
{
    return std::static_pointer_cast<const CompositeRectangle>
        (Rectangle::shared_from_this ());
}

CompositeRectangle::Ptr CompositeRectangle::shared_from_this ()
{
    return std::static_pointer_cast<CompositeRectangle>
        (Rectangle::shared_from_this ());
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

    g_assert (std::abs (new_height - child1 ()->height ()) < 0.001 &&
              std::abs (new_height - child2 ()->height ()) < 0.001);
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

    g_assert (std::abs (common_height - child1 ()->height ()) < 0.001 &&
              std::abs (common_height - child2 ()->height ()) < 0.001);
}

Rectangle::Ptr
HCompositeRectangle::find_rect (const double x, const double y)
{
    if (x < child1 ()->width ())
        return child1 ()->find_rect (x, y);

    return child2 ()->find_rect (x - child1 ()->width (), y);
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

    g_assert (std::abs (new_width - child1 ()->width ()) < 0.001 &&
              std::abs (new_width - child2 ()->width ()) < 0.001);
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

    g_assert (std::abs (common_width - child1 ()->width ()) < 0.001 &&
              std::abs (common_width - child2 ()->width ()) < 0.001);
}

Rectangle::Ptr
VCompositeRectangle::find_rect (const double x, const double y)
{
    if (y < child1 ()->height ())
        return child1 ()->find_rect (x, y);

    return child2 ()->find_rect (x, y - child1 ()->height ());
}
