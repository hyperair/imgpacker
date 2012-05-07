#include <queue>
#include <cmath>

#include <nihpp/sharedptrcreator.hh>
#include <imgpack/gtkui/collage-viewer.hh>
#include <imgpack/algorithm/bin-packer.hh>
#include <imgpack/util/logger.hh>

namespace ip = ImgPack;
namespace ipg = ip::GtkUI;
namespace ipa = ip::Algorithm;

namespace {
    class PixbufRectangle :
        public ipa::Rectangle,
        public nihpp::SharedPtrCreator<PixbufRectangle>
    {
    public:
        typedef std::shared_ptr<PixbufRectangle> Ptr;

        PixbufRectangle (const Glib::RefPtr<Gdk::Pixbuf> &pixbuf) :
            _pixbuf (pixbuf),
            _width (pixbuf->get_width ()),
            _height (pixbuf->get_height ())
        {
            LOG(info) << "Constructed PixbufRectangle with width: [" << _width
                      << "] and height: [" << _height << "]";
        }

        virtual ~PixbufRectangle (){}

        virtual double width () {return _width;}
        virtual void width (double);

        virtual double height () {return _height;}
        virtual void height (double);

        virtual double max_width () {return _pixbuf->get_width ();}
        virtual double max_height () {return _pixbuf->get_height ();}

        Glib::RefPtr<Gdk::Pixbuf> orig_pixbuf () const {return _pixbuf;}
        Glib::RefPtr<Gdk::Pixbuf> pixbuf () const;

    private:
        Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
        mutable Glib::RefPtr<Gdk::Pixbuf> scaled_pixbuf_cache;
        double _width;
        double _height;
    };
}

void PixbufRectangle::width (double new_width)
{
    if (std::abs (new_width - width ()) < 0.001)
        return;

    if (new_width - max_width () > 0.001) {
        LOG(error) << "Attempting to upscale pixbuf by width: "
                   << max_width () << " -> " << new_width;
        g_assert_not_reached ();
    }

    _height = new_width / aspect_ratio ();
    _width = std::min (new_width, max_width ());
}

void PixbufRectangle::height (double new_height)
{
    if (std::abs (new_height <= max_height ()) < 0.001)
        return;

    if (new_height - max_height () > 0.001) {
        LOG(error) << "Attempting to upscale pixbuf by height: "
                   << max_height () << " -> " << new_height;
        g_assert_not_reached ();
    }

    _width = new_height * aspect_ratio ();
    _height = std::min (new_height, max_height ());
}

inline Glib::RefPtr<Gdk::Pixbuf> PixbufRectangle::pixbuf () const
{
    int height = _height + 0.5;
    int width = _width + 0.5;

    if (!scaled_pixbuf_cache ||
        scaled_pixbuf_cache->get_height () != height ||
        scaled_pixbuf_cache->get_width () != width) {
        // HACK: Work around bug in gdk-pixbuf hanging when scaling large image
        // down.

        Glib::RefPtr<Gdk::Pixbuf> intermediate_pixbuf = _pixbuf;
        double intermediate_width = _pixbuf->get_width ();
        double intermediate_height = _pixbuf->get_height ();

        while (intermediate_width / width > 10) {
            intermediate_width /= 10;
            intermediate_height /= 10;

            intermediate_pixbuf =
                intermediate_pixbuf->scale_simple (intermediate_width,
                                                   intermediate_height,
                                                   Gdk::INTERP_BILINEAR);
        }

        scaled_pixbuf_cache =
            intermediate_pixbuf->scale_simple (width, height,
                                               Gdk::INTERP_BILINEAR);
    }

    return scaled_pixbuf_cache;
}


struct ipg::CollageViewer::Private : public sigc::trackable
{
    Private (ipg::CollageViewer &parent) :
        parent (parent), zoom_factor (1.0), dragging (false) {}

    CollageViewer       &parent;
    ipa::BinPacker::Ptr  packer;
    PixbufList           pixbufs;
    ipa::Rectangle::Ptr  collage;
    ipa::Rectangle::Ptr  selected;

    double               zoom_factor;
    bool                 dragging;

    void on_binpack_finish ();
};

void ipg::CollageViewer::Private::on_binpack_finish ()
{
    collage = packer->result ();
    selected.reset ();

    if (!collage)
        return;

    parent.set_size_request (collage->width () * zoom_factor,
                             collage->height () * zoom_factor);
    parent.queue_draw ();
}


ipg::CollageViewer::CollageViewer () :
    _priv (new Private (*this))
{
    add_events (Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::SMOOTH_SCROLL_MASK);
}

ipg::CollageViewer::~CollageViewer () {} // Needed for unique_ptr deleter

void ipg::CollageViewer::set_source_pixbufs (PixbufList pixbufs)
{
    _priv->pixbufs = std::move (pixbufs);

    refresh ();
}

void ipg::CollageViewer::refresh ()
{
    std::list<ipa::Rectangle::Ptr> rectangles;

    for (auto pixbuf : _priv->pixbufs)
        rectangles.push_back (PixbufRectangle::create (pixbuf));

    _priv->packer = ipa::BinPacker::create ();
    _priv->packer->connect_signal_finish
        (sigc::mem_fun (*_priv.get (), &Private::on_binpack_finish));
    _priv->packer->source_rectangles (std::move (rectangles));

    _priv->packer->start ();
}

void ipg::CollageViewer::reset ()
{
    _priv->packer.reset ();
    _priv->pixbufs.clear ();
}

void ipg::CollageViewer::export_to_file (const Glib::RefPtr<Gio::File> &file,
                                         const Gdk::PixbufFormat &format)
{
    int width = _priv->collage->width () + 1;
    int height = _priv->collage->height () + 1;

    auto surface = Cairo::ImageSurface::create (Cairo::FORMAT_RGB24,
                                                width, height);
    auto context = Cairo::Context::create (surface);

    on_draw (context);

    auto pixbuf = Gdk::Pixbuf::create (surface, 0, 0, width, height);

    gchar *buffer;
    gsize size;
    pixbuf->save_to_buffer (buffer, size, format.get_name ());

    auto stream = file->replace ();
    gsize bytes_written;
    stream->write_all (buffer, size, bytes_written);
}

namespace {
    struct RectangleCoord
    {
        ipa::Rectangle::Ptr rect;
        double x, y;

        RectangleCoord (ipa::Rectangle::Ptr rect, double x, double y) :
            rect (rect), x (x), y (y) {}
    };

    void draw_rect (const Cairo::RefPtr<Cairo::Context> &cr,
                    RectangleCoord rect)
    {
        std::queue<RectangleCoord> drawq;
        drawq.push (rect);

        while (!drawq.empty ()) {
            RectangleCoord rect = drawq.front ();
            drawq.pop ();

            double x = rect.x;
            double y = rect.y;

            if (rect.rect->orientation () == ipa::Rectangle::NONE) { // leaf
                PixbufRectangle::Ptr pixbufrect =
                    std::static_pointer_cast<PixbufRectangle> (rect.rect);

                cr->save ();
                Gdk::Cairo::set_source_pixbuf (cr, pixbufrect->pixbuf (),
                                               x, y);
                LOG(info) << "Drawing pixbuf " << pixbufrect->width ()
                          << ", " << pixbufrect->height ()
                          << " at " << x << ", " << y;

                cr->paint ();
                cr->restore ();

            } else {
                std::vector<ipa::Rectangle::Ptr> children =
                    {rect.rect->child1 (), rect.rect->child2 ()};

                for (auto i : children) {
                    if (!i)
                        continue;

                    drawq.push ({i, x, y});

                    switch (rect.rect->orientation ()) {
                    case ipa::Rectangle::HORIZONTAL:
                        x += i->width ();
                        break;

                    case ipa::Rectangle::VERTICAL:
                        y += i->height ();
                        break;

                    case ipa::Rectangle::NONE:
                        break;

                    default:
                        g_assert_not_reached ();
                    }
                }
            }
        }
    }
}

bool ipg::CollageViewer::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!_priv->collage)
        return true;

    cr->scale (_priv->zoom_factor, _priv->zoom_factor);

    draw_rect (cr, {_priv->collage, 0, 0});

    if (!_priv->selected)
        return true;

    // Begin drawing selection background + frame
    RectangleCoord selected = {_priv->selected, _priv->selected->offset_x (),
                               _priv->selected->offset_y ()};

    Glib::RefPtr<Gtk::StyleContext> context = get_style_context ();
    context->context_save ();
    cr->save ();

    context->add_class (GTK_STYLE_CLASS_VIEW);
    Gtk::StateFlags state = context->get_state ();
    state |= Gtk::STATE_FLAG_SELECTED;
    context->set_state (state);
    context->render_background (cr, selected.x - 4, selected.y - 4,
                                selected.rect->width () + 8,
                                selected.rect->height () + 8);
    context->render_frame (cr, selected.x - 4, selected.y - 4,
                           selected.rect->width () + 8,
                           selected.rect->height () + 8);
    cr->restore ();

    // Draw selection on top of background
    draw_rect (cr, selected);

    // Lightly draw background over selection again for better visibility
    auto bg_surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32,
                                                   selected.rect->width (),
                                                   selected.rect->height ());
    context->render_background (Cairo::Context::create (bg_surface),
                                0, 0,
                                selected.rect->width (),
                                selected.rect->height ());
    context->context_restore ();
    cr->save ();
    cr->set_source (bg_surface, selected.x, selected.y);
    cr->paint_with_alpha (0.2);
    cr->restore ();

    return true;
}

bool ipg::CollageViewer::on_button_release_event (GdkEventButton *ev)
{
    if (!_priv->collage)
        return true;

    if (_priv->dragging)
        return true;

    auto selection_leaf =
        _priv->collage->find_rect (ev->x / _priv->zoom_factor,
                                   ev->y / _priv->zoom_factor);
    bool found = false;

    for (auto i = selection_leaf; i; i = i->parent ())
        if (i == _priv->selected) {
            _priv->selected = i->parent ();
            found = true;
            break;
        }

    if (!found)
        _priv->selected = selection_leaf;

    if (_priv->selected) {
        std::vector<Gtk::TargetEntry> targets =
            {
                Gtk::TargetEntry ("application/x-imgpacker-rect",
                                  Gtk::TARGET_SAME_WIDGET)
            };

        drag_source_set (targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
        drag_dest_set (targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);

    } else {
        drag_source_unset ();
        drag_dest_unset ();
    }

    queue_draw ();

    return true;
}

bool ipg::CollageViewer::on_scroll_event (GdkEventScroll *ev)
{
    g_assert (ev->direction == GDK_SCROLL_SMOOTH);

    if (!(ev->state & GDK_CONTROL_MASK &&
          !(ev->state & GDK_SHIFT_MASK) && !(ev->state & GDK_META_MASK)))
        return false;

    _priv->zoom_factor *= std::pow (1.2, -ev->delta_y);
    set_size_request (_priv->collage->width () * _priv->zoom_factor + 1,
                      _priv->collage->height () * _priv->zoom_factor + 1);

    queue_draw ();

    return true;
}

void ipg::CollageViewer::on_drag_begin (const Glib::RefPtr<Gdk::DragContext> &)
{
    _priv->dragging = true;
    g_assert (_priv->selected);

    double width = _priv->selected->width ();
    double height = _priv->selected->height ();

    auto icon_surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32,
                                                     width,
                                                     height);
    auto cr = Cairo::Context::create (icon_surface);
    double factor = std::min (width / 120, height / 120);
    cr->scale (factor, factor);
    draw_rect (cr, {_priv->selected, 0, 0});

    auto icon_pixbuf = Gdk::Pixbuf::create (icon_surface, 0, 0, width, height);
    drag_source_set_icon (icon_pixbuf);
}

void ipg::CollageViewer::on_drag_end (const Glib::RefPtr<Gdk::DragContext>&)
{
    _priv->dragging = false;
}

bool ipg::CollageViewer::on_drag_drop (
    const Glib::RefPtr<Gdk::DragContext> &ctx,
    int x, int y,
    guint time)
{
    double real_x = x / _priv->zoom_factor;
    double real_y = y / _priv->zoom_factor;

    auto target = _priv->collage->find_rect (real_x, real_y);

    if (!target || !_priv->selected)
        return false;

    for (auto i = target; i; i = i->parent ())
        if (i == _priv->selected)
            return false;

    // detect closest edge in target rect
    // NB: this needs to be done before altering the tree
    double left_dist = real_x - target->offset_x ();
    double right_dist = target->offset_x () + target->width () - real_x;
    double top_dist = real_y - target->offset_y ();
    double bottom_dist = target->offset_y () + target->height () - real_y;

    g_assert (left_dist * right_dist * top_dist * bottom_dist >= 0);

    // We only accept dnd from the same widget, so just move the selected widget
    // to the target directly
    {
        // First remove selected from its original location
        auto parent = _priv->selected->parent ();
        if (!parent)
            return false;

        auto grandparent = parent->parent ();
        auto sibling = parent->child1 () == _priv->selected ?
            parent->child2 () : parent->child1 ();

        g_assert (sibling->parent () == parent && sibling != _priv->selected);
        g_assert ((parent->child1 () == sibling &&
                   parent->child2 () == _priv->selected) ||
                  (parent->child2 () == sibling &&
                   parent->child1 () == _priv->selected));

        // Unparent the current selection, as we're dropping that node
        _priv->selected->parent (nullptr);

        if (!grandparent) {
            _priv->collage = sibling;
            sibling->parent (nullptr);
        }

        else if (grandparent->child1 () == parent)
            grandparent->child1 (sibling);

        else
            grandparent->child2 (sibling);

        g_assert (sibling->parent () == grandparent);
        g_assert (!_priv->selected->parent ());
    }

    {
        // Now make selected and target siblings
        auto parent = target->parent ();
        g_assert (!parent ||
                  parent->child1 () == target ||
                  parent->child2 () == target);

        int position = !parent ? 0 : (parent->child1 () == target ? 1 : 2);

        target->parent (nullptr);

        double min_dist = std::min (std::min (left_dist, right_dist),
                                    std::min (top_dist, bottom_dist));

        ipa::CompositeRectangle::Ptr new_parent;
        if (min_dist == left_dist)
            new_parent = ipa::HCompositeRectangle::create (_priv->selected,
                                                           target);

        else if (min_dist == right_dist)
            new_parent = ipa::HCompositeRectangle::create (target,
                                                           _priv->selected);

        else if (min_dist == top_dist)
            new_parent = ipa::VCompositeRectangle::create (_priv->selected,
                                                           target);

        else
            new_parent = ipa::VCompositeRectangle::create (target,
                                                           _priv->selected);

        g_assert (target->parent () == new_parent);
        g_assert (_priv->selected->parent () == new_parent);

        if (!parent) {
            _priv->collage = new_parent;
            new_parent->parent (nullptr);

        } else if (position == 1)
            parent->child1 (new_parent);

        else
            parent->child2 (new_parent);

        g_assert (new_parent->parent () == parent);
        g_assert (!parent ||
                  (position == 1 && parent->child1 () == new_parent) ||
                  (position == 2 && parent->child2 () == new_parent));

        ipa::CompositeRectangle::Ptr root;
        for (auto i = parent; i; i = i->parent ())
            root = i;

        g_assert (root == _priv->collage);
        root->recalculate_size ();
    }

    ctx->drag_finish (true, true, time);
    set_size_request (_priv->collage->width () * _priv->zoom_factor,
                      _priv->collage->height () * _priv->zoom_factor);
    queue_draw ();

    return true;
}
