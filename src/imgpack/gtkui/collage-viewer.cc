#include <queue>

#include <nihpp/sharedptrcreator.hh>
#include <imgpack/gtkui/collage-viewer.hh>
#include <imgpack/bin-packer.hh>
#include <imgpack/logger.hh>

namespace ip = ImgPack;
namespace ipg = ip::GtkUI;

namespace {
    class PixbufRectangle :
        public ip::Rectangle,
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

        Glib::RefPtr<Gdk::Pixbuf> pixbuf () {return _pixbuf;}

    private:
        Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
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


struct ipg::CollageViewer::Private : public sigc::trackable
{
    Private (ipg::CollageViewer &parent) : parent (parent) {}

    CollageViewer &parent;
    BinPacker::Ptr packer;
    PixbufList     pixbufs;
    Rectangle::Ptr collage;

    void on_binpack_finish ();
};

void ipg::CollageViewer::Private::on_binpack_finish ()
{
    collage = packer->result ();
    parent.queue_draw ();
}


ipg::CollageViewer::CollageViewer () :
    _priv (new Private (*this)) {}

ipg::CollageViewer::~CollageViewer () {} // Needed for unique_ptr deleter

void ipg::CollageViewer::set_source_pixbufs (PixbufList pixbufs)
{
    _priv->pixbufs = std::move (pixbufs);

    refresh ();
}

void ipg::CollageViewer::refresh ()
{
    std::list<ip::Rectangle::Ptr> rectangles;

    for (auto pixbuf : _priv->pixbufs)
        rectangles.push_back (PixbufRectangle::create (pixbuf));

    _priv->packer = ip::BinPacker::create ();
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

namespace {
    struct RectangleCoord
    {
        ip::Rectangle::Ptr rect;
        double x, y;

        RectangleCoord (ip::Rectangle::Ptr rect, double x, double y) :
            rect (rect), x (x), y (y) {}
    };
}

bool ipg::CollageViewer::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!_priv->collage)
        return true;

    std::queue<RectangleCoord> drawq;
    drawq.push ({_priv->collage, 0, 0});

    while (!drawq.empty ()) {
        RectangleCoord rect = drawq.front ();
        drawq.pop ();

        double x = rect.x;
        double y = rect.y;

        auto children = rect.rect->children ();
        if (children.empty ()) {
            PixbufRectangle::Ptr pixbufrect =
                std::static_pointer_cast<PixbufRectangle> (rect.rect);

            cr->save ();

            Glib::RefPtr<Gdk::Pixbuf> scaled =
                pixbufrect->pixbuf ()->scale_simple (pixbufrect->width () + 0.5,
                                                     pixbufrect->height ()
                                                     + 0.5,
                                                     Gdk::INTERP_BILINEAR);
            Gdk::Cairo::set_source_pixbuf (cr, scaled, x, y);
            LOG(info) << "Drawing pixbuf " << pixbufrect->width ()
                      << ", " << pixbufrect->height ()
                      << " at " << x << ", " << y;

            cr->paint ();

            cr->restore ();

        } else {
            for (auto i : children) {
                drawq.push ({i, x, y});

                switch (rect.rect->orientation ()) {
                case ip::Rectangle::HORIZONTAL:
                    x += i->width ();
                    break;

                case ip::Rectangle::VERTICAL:
                    y += i->height ();
                    break;

                case ip::Rectangle::INVALID:
                    break;

                default:
                    g_assert_not_reached ();
                }
            }
        }
    }

    return true;
}
