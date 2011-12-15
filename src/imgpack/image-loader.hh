#ifndef IMGPACK_IMAGE_LOADER_HH
#define IMGPACK_IMAGE_LOADER_HH

#include <gtkmm.h>
#include <queue>

namespace ImgPack
{
    class ImageLoader
    {
    public:
        typedef sigc::slot<void,
                           Glib::RefPtr<Gio::File>,
                           Glib::RefPtr<Gdk::Pixbuf> > SlotFinish;
        typedef sigc::signal<void,
                             Glib::RefPtr<Gio::File>,
                             Glib::RefPtr<Gdk::Pixbuf> > SignalFinish;

        ImageLoader ();
        explicit ImageLoader (const SlotFinish &finish_handler);
        ~ImageLoader ();

        void push (const Glib::RefPtr<Gio::File> file);

        template <typename Iter>
        void push (Iter start, Iter end)
        {
            Glib::Mutex::Lock l (mutex);

            for (Iter i = start; i != end; i++)
                unlocked_push (*i);
        }

        SignalFinish &signal_finish () {return signal_finish_;}

    private:
        std::queue<Glib::RefPtr<Gio::File>> input_queue;
        std::queue<std::pair<Glib::RefPtr<Gio::File>,
                             Glib::RefPtr<Gdk::Pixbuf>>> output_queue;
        Glib::Mutex      mutex;
        Glib::Thread    *thread;

        Glib::Dispatcher dispatcher;
        SignalFinish     signal_finish_;

        void init ();
        void load_images ();
        void load_finish ();
        void unlocked_push (const Glib::RefPtr<Gio::File> file);
    };
}

#endif  // IMGPACK_IMAGE_LOADER_HH
