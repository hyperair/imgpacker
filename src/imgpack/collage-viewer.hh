#ifndef IMGPACK_COLLAGE_VIEWER_HH
#define IMGPACK_COLLAGE_VIEWER_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    class CollageViewer : public Gtk::DrawingArea
    {
    public:
        typedef std::list<Glib::RefPtr<Gdk::Pixbuf> > PixbufList;

        CollageViewer ();
        ~CollageViewer ();

        void set_source_pixbufs (PixbufList pixbufs);

        void refresh ();
        void reset ();

    protected:
        virtual bool on_draw (const Cairo::RefPtr<Cairo::Context> &cr);

    private:
        class Private;
        std::unique_ptr<Private> _priv;
    };
}

#endif  // IMGPACK_COLLAGE_VIEWER_HH
