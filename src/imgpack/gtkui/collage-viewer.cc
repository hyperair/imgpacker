#include <queue>

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

        virtual std::string description () {return "Pixbuf";}

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

    if (new_width > max_width ()) {
        LOG(error) << "Attempting to upscale pixbuf by width: "
                   << max_width () << " -> " << new_width;
        g_assert_not_reached ();
    }

    _height = new_width / aspect_ratio ();
    _width = new_width;
}

void PixbufRectangle::height (double new_height)
{
    if (std::abs (new_height <= max_height ()) < 0.001)
        return;

    if (new_height > max_height ()) {
        LOG(error) << "Attempting to upscale pixbuf by height: "
                   << max_height () << " -> " << new_height;
        g_assert_not_reached ();
    }

    _width = new_height * aspect_ratio ();
    _height = new_height;
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
    Private (ipg::CollageViewer &parent) : parent (parent) {}

    CollageViewer       &parent;
    ipa::BinPacker::Ptr  packer;
    PixbufList           pixbufs;
    ipa::Rectangle::Ptr  collage;
    ipa::Rectangle::Ptr  selected;

    sigc::signal<void, ipa::Rectangle::Ptr> signal_update;

    void on_binpack_finish ();
};

void ipg::CollageViewer::Private::on_binpack_finish ()
{
    collage = packer->result ();

    if (!collage)
        return;

    parent.set_size_request (collage->width () + 1, collage->height () + 1);
    parent.queue_draw ();
}


ipg::CollageViewer::CollageViewer () :
    _priv (new Private (*this))
{
    add_events (Gdk::BUTTON_PRESS_MASK);
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

sigc::connection ipg::CollageViewer::connect_signal_update (UpdateSlot slot)
{
    return _priv->signal_update.connect (slot);
}

namespace {
    struct RectangleCoord
    {
        ipa::Rectangle::Ptr rect;
        double x, y;

        RectangleCoord (ipa::Rectangle::Ptr rect, double x, double y) :
            rect (rect), x (x), y (y) {}
    };
}

bool ipg::CollageViewer::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!_priv->collage)
        return true;

    std::queue<RectangleCoord> drawq;
    drawq.push ({_priv->collage, 0, 0});

    double selected_x = -1;
    double selected_y = -1;
    double selected_width = -1;
    double selected_height = -1;

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

            if (rect.rect == _priv->selected) {
                selected_x = x;
                selected_y = y;
                selected_width = pixbufrect->width ();
                selected_height = pixbufrect->height ();
            }

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

    LOG(info) << "Stroking selected rectangle: "
              << selected_x << ", " << selected_y << "; "
              << selected_width << "x" << selected_height;
    cr->save ();
    cr->rectangle (selected_x, selected_y, selected_width, selected_height);
    cr->set_source_rgb (0, 1, 1);
    cr->set_line_width (4);
    cr->stroke ();
    cr->restore ();


    return true;
}

bool ipg::CollageViewer::on_button_press_event (GdkEventButton *ev)
{
    if (!_priv->collage)
        return true;

    _priv->selected = _priv->collage->find_rect (ev->x, ev->y);
    LOG(info) << "Click at " << ev->x << ", " << ev->y;
    queue_draw ();

    return true;
}
