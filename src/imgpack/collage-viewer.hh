#ifndef IMGPACK_COLLAGE_VIEWER_HH
#define IMGPACK_COLLAGE_VIEWER_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    class CollageViewer : public Gtk::DrawingArea
    {
    public:
        typedef std::shared_ptr<CollageViewer> Ptr;
        typedef std::weak_ptr<CollageViewer> WPtr;

        static Ptr create ();

        CollageViewer () = default;
        virtual ~CollageViewer () {}

        virtual void
        set_source_pixbufs
        (const std::list<Glib::RefPtr<Gdk::Pixbuf> > &pixbufs) = 0;

        virtual void refresh () = 0;
        virtual void reset () = 0;
    };
}

#endif  // IMGPACK_COLLAGE_VIEWER_HH
