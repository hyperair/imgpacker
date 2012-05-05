#ifndef IMGPACK_COLLAGE_VIEWER_HH
#define IMGPACK_COLLAGE_VIEWER_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    namespace GtkUI
    {
        class CollageViewer : public Gtk::DrawingArea
        {
        public:
            typedef std::vector<Glib::RefPtr<Gdk::Pixbuf> > PixbufList;

            CollageViewer ();
            ~CollageViewer ();

            void set_source_pixbufs (PixbufList pixbufs);

            void refresh ();
            void reset ();

            void export_to_file (const Glib::RefPtr<Gio::File> &file,
                                 const Gdk::PixbufFormat &format);

        protected:
            virtual bool on_draw (const Cairo::RefPtr<Cairo::Context> &cr);
            virtual bool on_button_press_event (GdkEventButton *ev);

        private:
            class Private;
            std::unique_ptr<Private> _priv;
        };
    }
}

#endif  // IMGPACK_COLLAGE_VIEWER_HH
