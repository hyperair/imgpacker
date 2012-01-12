#ifndef _IMGPACK_PIXBUF_LOADER_HH
#define _IMGPACK_PIXBUF_LOADER_HH

#include <queue>
#include <unordered_set>

#include <nihpp/sharedptrcreator.hh>
#include <gtkmm.h>

namespace ImgPack
{
    class MainWindow;
    class StatusClient;

    class PixbufLoader :
        public nihpp::SharedPtrCreator<PixbufLoader>,
        public sigc::trackable
    {
    private:
        class Result;

    public:
        PixbufLoader (std::shared_ptr<StatusClient> status);
        ~PixbufLoader () {abort ();}

        void enqueue (const Glib::RefPtr<Gio::File> &file);
        void start ();
        void abort ();

        const std::list<std::shared_ptr<Result> > &results () {return _results;}

        sigc::connection connect_signal_finish (const sigc::slot<void> &slot)
        {
            return finish.connect (slot);
        }

        sigc::connection connect_signal_abort (const sigc::slot<void> &slot)
        {
            return aborted.connect (slot);
        }

    private:
        std::shared_ptr<StatusClient> status;
        sigc::signal<void> finish;
        sigc::signal<void> aborted;

        Glib::Thread *worker;

        Glib::Dispatcher thread_finish;
        Glib::Dispatcher progress;

        guint status_context;

        Glib::Mutex mutex;
        std::queue<Glib::RefPtr<Gio::File> > unprocessed;
        std::list<std::shared_ptr<Result> > _results;

        std::unordered_set<std::string> visited;

        Glib::RefPtr<Gio::Cancellable> cancellable;

        // private functions
        void _worker ();
        void testcancelled ();
        Glib::RefPtr<Gio::File> get_next_unprocessed ();
        void recurse_file (const Glib::RefPtr<Gio::File> &file);
        void load_pixbuf (const Glib::RefPtr<Gio::File> &file);

        void on_thread_finish ();
        void on_progress ();
    };


    // Result class for PixbufLoader
    class PixbufLoader::Result :
        public nihpp::SharedPtrCreator<Result>
    {
    public:
        Result (Glib::RefPtr<Gio::File> file,
                Glib::RefPtr<Gdk::Pixbuf> pixbuf) :
            _file (file),
            _pixbuf (pixbuf),
            error (false)
        {}

        Result (Glib::RefPtr<Gio::File> file,
                const Glib::Exception &exception) :
            _file (file),
            _message (exception.what ()),
            error (true)
        {}

        Glib::RefPtr<Gio::File>     file ()     {return _file;}
        Glib::RefPtr<Gdk::Pixbuf>   pixbuf ()   {return _pixbuf;}
        Glib::ustring               message ()  {return _message;}

        explicit operator bool () {return !error;}

    private:
        Glib::RefPtr<Gio::File>     _file;
        Glib::RefPtr<Gdk::Pixbuf>   _pixbuf;
        Glib::ustring               _message;

        bool error;
    };
}

#endif  // _IMGPACK_PIXBUF_LOADER_HH
