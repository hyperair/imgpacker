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
            virtual bool on_button_release_event (GdkEventButton *ev);
            virtual bool on_scroll_event (GdkEventScroll *ev);
            virtual void on_drag_begin (const Glib::RefPtr<Gdk::DragContext> &);
            virtual void on_drag_end (const Glib::RefPtr<Gdk::DragContext> &);
            virtual bool on_drag_drop (const Glib::RefPtr<Gdk::DragContext> &,
                                       int x, int y, guint time);


        private:
            class Private;
            std::unique_ptr<Private> _priv;
        };
    }
}

#endif  // IMGPACK_COLLAGE_VIEWER_HH
