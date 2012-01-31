#include <unordered_set>
#include <queue>

#include <autosprintf.h>

#include <glibmm/i18n.h>
#include <imgpack/pixbuf-loader.hh>
#include <imgpack/main-window.hh>
#include <imgpack/logger.hh>

using ImgPack::PixbufLoader;
using ImgPack::StatusClient;

namespace {
    class PixbufLoaderImpl :
        public PixbufLoader,
        public nihpp::SharedPtrCreator<PixbufLoaderImpl>
    {
    public:
        using nihpp::SharedPtrCreator<PixbufLoaderImpl>::Ptr;
        using nihpp::SharedPtrCreator<PixbufLoaderImpl>::WPtr;
        using nihpp::SharedPtrCreator<PixbufLoaderImpl>::create;

        PixbufLoaderImpl (const std::shared_ptr<StatusClient> &status);
        virtual ~PixbufLoaderImpl () {abort ();}

        virtual void enqueue (const Glib::RefPtr<Gio::File> &file);

        virtual const std::list<std::shared_ptr<Result> > &results () const
        {return _results;}

    private:
        StatusClient::Ptr status;

        Glib::Dispatcher progress;

        guint status_context;

        Glib::Mutex mutex;
        std::queue<Glib::RefPtr<Gio::File> > unprocessed;
        std::list<std::shared_ptr<Result> > _results;

        std::unordered_set<std::string> visited;

        Glib::RefPtr<Gio::Cancellable> cancellable;

        // private functions
        virtual void run ();

        Glib::RefPtr<Gio::File> get_next_unprocessed ();
        void recurse_file (const Glib::RefPtr<Gio::File> &file);
        void load_pixbuf (const Glib::RefPtr<Gio::File> &file);

        void on_progress ();
    };
}

// static
PixbufLoader::Ptr PixbufLoader::create (const StatusClient::Ptr &status)
{
    return PixbufLoaderImpl::create (status);
}

PixbufLoaderImpl::PixbufLoaderImpl (const StatusClient::Ptr &status) :
    status (status),
    status_context (status->statusbar ().get_context_id ("PixbufLoader"))
{
    progress.connect (sigc::mem_fun (*this, &PixbufLoaderImpl::on_progress));
}

void PixbufLoaderImpl::enqueue (const Glib::RefPtr<Gio::File> &file)
{
    unprocessed.push (file);
}

Glib::RefPtr<Gio::File> PixbufLoaderImpl::get_next_unprocessed ()
{
    Glib::Mutex::Lock l (mutex);

    if (unprocessed.empty ())
        return Glib::RefPtr<Gio::File> ();

    auto retval = unprocessed.front ();
    unprocessed.pop ();

    return retval;
}

void PixbufLoaderImpl::run ()
{
    while (Glib::RefPtr<Gio::File> file = get_next_unprocessed ()) {
        try {
            auto fileinfo = file->query_info (cancellable);
            testcancelled ();

            std::string fileid =
                fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE);

            // Check if visited to avoid recursive loop
            if (visited.find (fileid) != visited.end ())
                    continue;

            switch (fileinfo->get_file_type ()) {
            case Gio::FILE_TYPE_DIRECTORY:
                recurse_file (file);
                break;

            case Gio::FILE_TYPE_REGULAR:
                load_pixbuf (file);
                break;

            default:
                LOG(warning) << "Ignoring file " << file->get_uri ()
                             << " because it has unknown file type "
                             << fileinfo->get_file_type ();

            }

            visited.insert
                (fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE));

        } catch (Gio::Error &e) {
            LOG(warning) << "Skipping file " << file->get_uri ()
                         << "because Gio::Error was thrown: "
                         << e.what ();

        } catch (Glib::Thread::Exit &e) {
            // We were cancelled, so put the file back
            Glib::Mutex::Lock l (mutex);
            unprocessed.push (file);

            throw;
        }

        testcancelled ();
    }
}

void PixbufLoaderImpl::recurse_file (const Glib::RefPtr<Gio::File> &file)
{
    // Enqueue all children to the back (BFS)
    Glib::RefPtr<Gio::FileEnumerator> fe =
        file->enumerate_children
        (cancellable, "*",
         Gio::FILE_QUERY_INFO_NOFOLLOW_SYMLINKS);

    testcancelled ();

    while (Glib::RefPtr<Gio::FileInfo> i =
           fe->next_file (cancellable)) {
        Glib::RefPtr<Gio::File> child =
            file->resolve_relative_path (i->get_name ());

        enqueue (child);
    }
}

void PixbufLoaderImpl::load_pixbuf (const Glib::RefPtr<Gio::File> &file)
{
    try {
        auto pixbuf =
            Gdk::Pixbuf::create_from_stream (file->read (), cancellable);

        testcancelled ();

        Glib::Mutex::Lock l (mutex);

        _results.push_back (Result::create (file, pixbuf));

        LOG(info) << "Successfully loaded pixbuf from " << file->get_uri ();

    } catch (Glib::Exception &e) {
        testcancelled ();

        _results.push_back (Result::create (file, e));

        LOG(info) << "Could not load pixbuf from " << file->get_uri () << ": "
                  << e.what ();
    }

    progress ();
}

void PixbufLoaderImpl::on_progress ()
{
    int unprocessed_size = unprocessed.size ();
    int results_size = _results.size ();

    int total = unprocessed_size + results_size;
    double fraction;

    if (total == 0)
        fraction = 0;

    else
        fraction = (double)results_size / total;

    status->statusbar ().pop (status_context);

    std::string message = gnu::autosprintf (_("Adding images (%d/%d)"),
                                            results_size, total);
    status->statusbar ().push (message, status_context);

    LOG(info) << "Progress fraction is " << fraction;
    status->progressbar ().set_fraction (fraction);
}
