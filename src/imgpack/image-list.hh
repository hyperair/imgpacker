#ifndef IMGPACK_IMAGE_LIST_HH
#define IMGPACK_IMAGE_LIST_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    class Application;

    class ImageList : public Gtk::IconView
    {
    public:
        ImageList (Application &app);
        ~ImageList () {}

        void add_image (const Glib::RefPtr<Gio::File> &file,
                        Glib::RefPtr<Gdk::Pixbuf> pixbuf =
                        Glib::RefPtr<Gdk::Pixbuf>());

        typedef sigc::slot<void, Glib::RefPtr<Gio::File>> SlotFinish;
        void add_image_async (const Glib::RefPtr<Gio::File> &file);
        void remove_selected ();

    private:
        Glib::RefPtr<Gtk::ListStore> model;
        Application &app;

        class LoaderTask;
        std::list<std::shared_ptr<LoaderTask>> load_queue;

        Glib::Dispatcher image_ready;

        void on_image_ready ();
    };
}

#endif  // IMGPACK_IMAGE_LIST_HH
