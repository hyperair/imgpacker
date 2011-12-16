#ifndef IMGPACK_IMAGE_LIST_HH
#define IMGPACK_IMAGE_LIST_HH

#include <gtkmm.h>

namespace ImgPack
{
    class ImageList : public Gtk::IconView
    {
    public:
        ImageList ();
        ~ImageList () {}

        void add_image (const Glib::RefPtr<Gio::File> &file,
                        Glib::RefPtr<Gdk::Pixbuf> pixbuf =
                        Glib::RefPtr<Gdk::Pixbuf>());

        typedef sigc::slot<void, Glib::RefPtr<Gio::File>> SlotFinish;
        void add_image_async (const Glib::RefPtr<Gio::File> &file,
                              SlotFinish finish_handler);
        void remove_selected ();

    private:
        Glib::RefPtr<Gtk::ListStore> model;
    };
}

#endif  // IMGPACK_IMAGE_LIST_HH
