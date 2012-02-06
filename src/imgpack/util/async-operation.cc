#include <imgpack/util/async-operation.hh>
#include <imgpack/util/logger.hh>

namespace ip = ImgPack;
namespace ipu = ImgPack::Util;

using ipu::AsyncOperation;

struct AsyncOperation::Private : sigc::trackable
{
    explicit Private (std::string &&description);

    const std::string description;

    Glib::Thread *thread;
    Glib::RefPtr<Gio::Cancellable> cancellable;

    Glib::Dispatcher thread_finish;
    sigc::signal<void> finished;
    sigc::signal<void> aborted;

    void cleanup_thread ();
    void on_thread_finish ();
};


// AsyncOperation definitions
AsyncOperation::AsyncOperation (std::string description) :
    _priv (new Private (std::move (description)))
{
    connect_signal_finish (sigc::mem_fun (*this, &AsyncOperation::on_finish));
    connect_signal_abort (sigc::mem_fun (*this, &AsyncOperation::on_abort));
}

AsyncOperation::~AsyncOperation () {abort ();}

namespace {
    class CleanupHelper
    {
    public:
        CleanupHelper (std::function<void()> hook) :
            hook (std::move (hook)), cleanup (true) {}
        ~CleanupHelper () {if (cleanup) hook ();}

        // non-copyable
        CleanupHelper (const CleanupHelper &) = delete;
        CleanupHelper &operator= (const CleanupHelper &) = delete;

        void clear () {cleanup = false;}

    private:
        std::function<void()> hook;
        bool cleanup;
    };
}

void AsyncOperation::start ()
{
    g_assert (!_priv->thread);

    LOG(info) << "Starting async process: " << _priv->description;

    _priv->thread = Glib::Thread::create (
        [=] ()
        {
            CleanupHelper ch ([=] () {_priv->thread_finish ();});

            try {
                run ();

            } catch (Cancelled &e) {
                // Async operation was aborted, so do not send finish signal
                ch.clear ();
            }
        }, true);
}

void AsyncOperation::abort ()
{
    if (!is_running ())
        return;

    LOG(info) << "Aborting async process: " << _priv->description;
    _priv->cancellable->cancel ();

    _priv->cleanup_thread ();

    Glib::signal_idle ().connect_once (_priv->aborted);
}

bool AsyncOperation::is_running ()
{
    return _priv->thread;
}

sigc::connection
AsyncOperation::connect_signal_finish (sigc::slot<void> finish_slot)
{
    return _priv->finished.connect (finish_slot);
}

sigc::connection
AsyncOperation::connect_signal_abort (sigc::slot<void> abort_slot)
{
    return _priv->aborted.connect (abort_slot);
}

Glib::RefPtr<Gio::Cancellable> AsyncOperation::cancellable ()
{
    return _priv->cancellable;
}

void AsyncOperation::testcancelled ()
{
    if (_priv->cancellable->is_cancelled ())
        throw Cancelled ();
}


// AsyncOperation::Cancelled definitions
const char *AsyncOperation::Cancelled::what () const throw ()
{
    return "Async operation cancelled";
}


// AsyncOperation::Private definitions
AsyncOperation::Private::Private (std::string &&description) :
    description (description),
    thread (nullptr),
    cancellable (Gio::Cancellable::create ())
{
    thread_finish.connect (sigc::mem_fun (*this, &Private::on_thread_finish));
}

void AsyncOperation::Private::cleanup_thread ()
{
    g_assert (thread);

    thread->join ();
    thread = nullptr;
}

void AsyncOperation::Private::on_thread_finish ()
{
    cleanup_thread ();
    Glib::signal_idle ().connect_once (finished);
}
