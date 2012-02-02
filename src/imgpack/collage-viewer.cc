#include <queue>

#include <nihpp/sharedptrcreator.hh>
#include <imgpack/collage-viewer.hh>
#include <imgpack/bin-packer.hh>
#include <imgpack/logger.hh>

namespace ip = ImgPack;

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


    class CollageViewerImpl :
        public ip::CollageViewer,
        public nihpp::SharedPtrCreator<CollageViewerImpl>
    {
    public:
        using nihpp::SharedPtrCreator<CollageViewerImpl>::create;
        typedef std::list<Glib::RefPtr<Gdk::Pixbuf> > PixbufList;

        CollageViewerImpl () {}
        virtual ~CollageViewerImpl () {}

        virtual void set_source_pixbufs (PixbufList pixbufs);

        virtual void refresh ();
        virtual void reset ();

    private:
        PixbufList pixbufs;
        ip::BinPacker::Ptr packer;
        ip::Rectangle::Ptr collage;

        virtual bool on_draw (const Cairo::RefPtr <Cairo::Context> &cr);
        virtual void on_binpack_finish ();
    };
}

void PixbufRectangle::width (double new_width)
{
    if (new_width == width ())
        return;

    g_assert (new_width <= max_width ());

    _height = new_width / aspect_ratio ();
    _width = new_width;
}

void PixbufRectangle::height (double new_height)
{
    if (new_height <= max_height ())
        return;

    g_assert (new_height <= max_height ());

    _width = new_height * aspect_ratio ();
    _height = new_height;
}

ip::CollageViewer::Ptr ip::CollageViewer::create ()
{
    return CollageViewerImpl::create ();
}

void
CollageViewerImpl::set_source_pixbufs (PixbufList pixbufs)
{
    this->pixbufs = std::move (pixbufs);

    refresh ();
}

void CollageViewerImpl::refresh ()
{
    std::list<ip::Rectangle::Ptr> rectangles;

    for (auto pixbuf : pixbufs)
        rectangles.push_back (PixbufRectangle::create (pixbuf));

    packer = ip::BinPacker::create ();
    packer->connect_signal_finish
        (sigc::mem_fun (*this, &CollageViewerImpl::on_binpack_finish));
    packer->source_rectangles (std::move (rectangles));

    packer->start ();
}

void CollageViewerImpl::reset ()
{
    packer.reset ();
    pixbufs.clear ();
}

namespace {
    struct RectangleCoord
    {
        ip::Rectangle::Ptr rect;
        int x, y;

        RectangleCoord (ip::Rectangle::Ptr rect, int x, int y) :
            rect (rect), x (x), y (y) {}
    };
}

bool CollageViewerImpl::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!collage)
        return true;

    std::queue<RectangleCoord> drawq;
    drawq.push ({collage, 0, 0});

    while (!drawq.empty ()) {
        RectangleCoord rect = drawq.front ();
        drawq.pop ();

        int x = rect.x;
        int y = rect.y;

        auto children = rect.rect->children ();
        if (children.empty ()) {
            PixbufRectangle::Ptr pixbufrect =
                std::static_pointer_cast<PixbufRectangle> (rect.rect);

            cr->save ();

            Glib::RefPtr<Gdk::Pixbuf> scaled =
                pixbufrect->pixbuf ()->scale_simple (pixbufrect->width (),
                                                     pixbufrect->height (),
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

void CollageViewerImpl::on_binpack_finish ()
{
    collage = packer->result ();
    queue_draw ();
}
