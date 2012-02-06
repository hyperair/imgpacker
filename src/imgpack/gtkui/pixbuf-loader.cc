#include <unordered_set>
#include <queue>

#include <autosprintf.h>

#include <glibmm/i18n.h>
#include <imgpack/gtkui/pixbuf-loader.hh>
#include <imgpack/gtkui/main-window.hh>
#include <imgpack/util/logger.hh>

namespace ip = ImgPack;
namespace ipg = ip::GtkUI;

using ipg::PixbufLoader;

struct PixbufLoader::Private
{
    Private (PixbufLoader &self, const ipg::StatusClient::Ptr &status);

    PixbufLoader &self;
    StatusClient::Ptr status;

    Glib::Dispatcher progress;

    guint status_context;

    Glib::Mutex mutex;
    std::queue<Glib::RefPtr<Gio::File> > unprocessed;
    std::list<std::shared_ptr<Result> > results;

    std::unordered_set<std::string> visited;

    Glib::RefPtr<Gio::File> get_next_unprocessed ();
    void recurse_file (const Glib::RefPtr<Gio::File> &file);
    void load_pixbuf (const Glib::RefPtr<Gio::File> &file);

    void on_progress ();

    Glib::RefPtr<Gio::Cancellable> cancellable () {return self.cancellable ();}
    void testcancelled () {self.testcancelled ();}
};

PixbufLoader::Private::Private (PixbufLoader &self,
                                const ipg::StatusClient::Ptr &status) :
    self (self),
    status (status),
    status_context (status->statusbar ().get_context_id ("PixbufLoader")) {}

Glib::RefPtr<Gio::File> PixbufLoader::Private::get_next_unprocessed ()
{
    Glib::Mutex::Lock l (mutex);

    if (unprocessed.empty ())
        return Glib::RefPtr<Gio::File> ();

    auto retval = unprocessed.front ();
    unprocessed.pop ();

    return retval;
}

void PixbufLoader::Private::recurse_file (const Glib::RefPtr<Gio::File> &file)
{
    // Enqueue all children to the back (BFS)
    Glib::RefPtr<Gio::FileEnumerator> fe =
        file->enumerate_children
        (cancellable (), "*",
         Gio::FILE_QUERY_INFO_NOFOLLOW_SYMLINKS);

    testcancelled ();

    while (Glib::RefPtr<Gio::FileInfo> i = fe->next_file (cancellable ())) {
        Glib::RefPtr<Gio::File> child =
            file->resolve_relative_path (i->get_name ());

        self.enqueue (child);
    }
}

void PixbufLoader::Private::load_pixbuf (const Glib::RefPtr<Gio::File> &file)
{
    try {
        auto pixbuf =
            Gdk::Pixbuf::create_from_stream (file->read (),
                                             cancellable ());

        testcancelled ();

        Glib::Mutex::Lock l (mutex);

        results.push_back (Result::create (file, pixbuf));

        LOG(info) << "Successfully loaded pixbuf from " << file->get_uri ();

    } catch (Glib::Exception &e) {
        testcancelled ();

        results.push_back (Result::create (file, e));

        LOG(info) << "Could not load pixbuf from " << file->get_uri () << ": "
                  << e.what ();
    }

    progress ();
}

void PixbufLoader::Private::on_progress ()
{
    int unprocessed_size = unprocessed.size ();
    int results_size = results.size ();

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


// PixbufLoader definitions
PixbufLoader::PixbufLoader (const ipg::StatusClient::Ptr &status) :
    AsyncOperation ("PixbufLoader"),
    _priv (new Private (*this, status))
{
    _priv->progress.connect (sigc::mem_fun (*_priv, &Private::on_progress));
}

PixbufLoader::~PixbufLoader () {}

void PixbufLoader::enqueue (const Glib::RefPtr<Gio::File> &file)
{
    _priv->unprocessed.push (file);
}

const std::list<PixbufLoader::Result::Ptr> &PixbufLoader::results () const
{
    return _priv->results;
}

void PixbufLoader::run ()
{
    while (Glib::RefPtr<Gio::File> file = _priv->get_next_unprocessed ()) {
        try {
            auto fileinfo = file->query_info (cancellable ());
            testcancelled ();

            std::string fileid =
                fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE);

            // Check if visited to avoid recursive loop
            if (_priv->visited.find (fileid) != _priv->visited.end ())
                    continue;

            switch (fileinfo->get_file_type ()) {
            case Gio::FILE_TYPE_DIRECTORY:
                _priv->recurse_file (file);
                break;

            case Gio::FILE_TYPE_REGULAR:
                _priv->load_pixbuf (file);
                break;

            default:
                LOG(warning) << "Ignoring file " << file->get_uri ()
                             << " because it has unknown file type "
                             << fileinfo->get_file_type ();
            }

            _priv->visited.insert
                (fileinfo->get_attribute_string (G_FILE_ATTRIBUTE_ID_FILE));

        } catch (Gio::Error &e) {
            LOG(warning) << "Skipping file " << file->get_uri ()
                         << "because Gio::Error was thrown: "
                         << e.what ();

        } catch (Cancelled &e) {
            // We were cancelled, so put the file back
            Glib::Mutex::Lock l (_priv->mutex);
            _priv->unprocessed.push (file);

            throw;
        }

        testcancelled ();
    }
}
