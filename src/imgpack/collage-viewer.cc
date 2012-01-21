#include <nihpp/sharedptrcreator.hh>
#include <imgpack/collage-viewer.hh>
#include <imgpack/bin-packer.hh>
#include <imgpack/logger.hh>

namespace ip = ImgPack;

namespace {
    class PixbufRectangle : public ip::Rectangle
    {
    public:
        PixbufRectangle (const Glib::RefPtr<Gdk::Pixbuf> &pixbuf) :
            _pixbuf (pixbuf),
            _width (pixbuf->get_width ()),
            _height (pixbuf->get_height ())
        {}

        virtual ~PixbufRectangle (){}

        virtual int width () {return _width;}
        virtual int height () {return _height;}

        virtual int max_width () {return _pixbuf->get_width ();}
        virtual int max_height () {return _pixbuf->get_height ();}

    private:
        Glib::RefPtr<Gdk::Pixbuf> _pixbuf;
        int _width;
        int _height;
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

        virtual void set_source_pixbufs (const PixbufList &pixbufs);

        virtual void refresh ();
        virtual void reset ();

        virtual bool on_draw (const Cairo::RefPtr <Cairo::Context> &cr);

    private:
        PixbufList pixbufs;
        ip::BinPacker::Ptr packer;
    };
}

ip::CollageViewer::Ptr ip::CollageViewer::create ()
{
    return CollageViewerImpl::create ();
}

void
CollageViewerImpl::set_source_pixbufs (const PixbufList &)
{
    // Use algorithm to generate pixbufs
    refresh ();
}

void CollageViewerImpl::refresh ()
{
}

void CollageViewerImpl::reset ()
{
    packer.reset ();
    pixbufs.clear ();
}

bool CollageViewerImpl::on_draw (const Cairo::RefPtr<Cairo::Context> &)
{
    return false;
}
