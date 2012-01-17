#include <nihpp/sharedptrcreator.hh>
#include <imgpack/collage-viewer.hh>
#include <imgpack/bin-packer.hh>
#include <imgpack/logger.hh>

using namespace ImgPack;

namespace {
    class CollageViewerImpl :
        public CollageViewer,
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
        BinPacker::Ptr packer;
    };
}

CollageViewer::Ptr CollageViewer::create ()
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
}

bool CollageViewerImpl::on_draw (const Cairo::RefPtr<Cairo::Context> &)
{
    return false;
}
