#ifndef IMGPACK_IMAGE_LIST_HH
#define IMGPACK_IMAGE_LIST_HH

#include <gtkmm.h>

namespace ImgPack
{
    class ImageList : public Gtk::IconView
    {
    public:
        ImageList ();
        ImageList (const ImageList &) = delete;
        ~ImageList () {}

        void add_image (const Glib::RefPtr<Gio::File> &file,
                        const Glib::RefPtr<Gdk::Pixbuf> &pixbuf);

        void remove_selected ();

    private:
        Glib::RefPtr<Gtk::ListStore> model;
    };
}

#endif  // IMGPACK_IMAGE_LIST_HH
