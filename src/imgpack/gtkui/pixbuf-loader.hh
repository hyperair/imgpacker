#ifndef _IMGPACK_PIXBUF_LOADER_HH
#define _IMGPACK_PIXBUF_LOADER_HH

#include <nihpp/sharedptrcreator.hh>
#include <gtkmm.h>

#include <imgpack/async-operation.hh>

namespace ImgPack
{
    namespace GtkUI
    {
        class MainWindow;
        class StatusClient;

        class PixbufLoader : public AsyncOperation,
                             public nihpp::SharedPtrCreator<PixbufLoader>
        {
        public:
            class Result;

            PixbufLoader (const std::shared_ptr<StatusClient> &status);
            PixbufLoader (const PixbufLoader &) = delete;
            ~PixbufLoader();

            void enqueue (const Glib::RefPtr<Gio::File> &file);

            const std::list<std::shared_ptr<Result> > & results () const;

        private:
            class Private;
            std::unique_ptr<Private> _priv;

            virtual void run ();
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
}

#endif  // _IMGPACK_PIXBUF_LOADER_HH
