#include <string>
#include <iostream>
// The above two includes are needed before autosprintf.h because it does funny
// things with __attribute__
#include <autosprintf.h>

#include <glibmm/i18n.h>
#include <imgpack/pixbuf-loader.hh>
#include <imgpack/main-window.hh>
#include <imgpack/logger.hh>

using ImgPack::PixbufLoader;

PixbufLoader::PixbufLoader (StatusClient::Ptr status) :
    status (status),
    worker (nullptr),
    status_context (status->statusbar ().get_context_id ("PixbufLoader")),
    cancellable (Gio::Cancellable::create ())
{
    thread_finish.connect (sigc::mem_fun (*this,
                                          &PixbufLoader::on_thread_finish));
    progress.connect (sigc::mem_fun (*this, &PixbufLoader::on_progress));
}

void PixbufLoader::enqueue (const Glib::RefPtr<Gio::File> &file)
{
    unprocessed.push (file);
}

void PixbufLoader::start ()
{
    g_assert (!worker);

    worker = Glib::Thread::create
        (sigc::mem_fun (*this, &PixbufLoader::_worker), true);
}

void PixbufLoader::abort ()
{
    if (!worker)
        return;

    cancellable->cancel ();
    worker->join ();

    finish ();
}

namespace {
    struct CleanupHelper
    {
        CleanupHelper (Glib::Dispatcher &dispatcher) :
            dispatcher (dispatcher) {}
        ~CleanupHelper () {dispatcher ();}

        Glib::Dispatcher &dispatcher;
    };
}

Glib::RefPtr<Gio::File> PixbufLoader::get_next_unprocessed ()
{
    Glib::Mutex::Lock l (mutex);

    if (unprocessed.empty ())
        return Glib::RefPtr<Gio::File> ();

    auto retval = unprocessed.front ();
    unprocessed.pop ();

    return retval;
}

void PixbufLoader::_worker ()
{
    CleanupHelper cleanup (thread_finish);

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

void PixbufLoader::testcancelled ()
{
    if (cancellable->is_cancelled ())
        throw Glib::Thread::Exit ();
}

void PixbufLoader::recurse_file (const Glib::RefPtr<Gio::File> &file)
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

void PixbufLoader::load_pixbuf (const Glib::RefPtr<Gio::File> &file)
{
    try {
        auto pixbuf =
            Gdk::Pixbuf::create_from_stream (file->read ());

        Glib::Mutex::Lock l (mutex);

        _results.push_back (Result::create (file, pixbuf));

        LOG(info) << "Successfully loaded pixbuf from " << file->get_uri ();

    } catch (Glib::Exception &e) {
        _results.push_back (Result::create (file, e));

        LOG(info) << "Could not load pixbuf from " << file->get_uri () << ": "
                  << e.what ();
    }

    progress ();
}

void PixbufLoader::on_thread_finish ()
{
    if (!worker)                // This can happen because abort() was called
        return;

    worker->join ();
    worker = nullptr;

    finish ();
}

void PixbufLoader::on_progress ()
{
    int unprocessed_size = unprocessed.size ();
    int results_size = _results.size ();

    int total = unprocessed_size + results_size;
    double fraction;

    if (total == 0)
        fraction = 0;

    else
        fraction = (double)results_size / unprocessed_size;

    status->statusbar ().pop (status_context);

    std::string message = gnu::autosprintf (_("Adding images (%d/%d)"),
                                            results_size, total);
    status->statusbar ().push (message, status_context);

    status->progressbar ().set_fraction (fraction);
}
