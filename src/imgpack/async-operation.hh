#ifndef _IMGPACK_ASYNC_OPERATION_HH
#define _IMGPACK_ASYNC_OPERATION_HH

#include <memory>
#include <string>

#include <glibmm.h>
#include <giomm.h>

namespace ImgPack
{
    class AsyncOperation : public sigc::trackable
    {
    public:
        AsyncOperation (std::string description = "{anonymous}");
        virtual ~AsyncOperation ();

        AsyncOperation (const AsyncOperation &) = delete;
        AsyncOperation &operator= (const AsyncOperation &) = delete;

        void start ();
        void abort ();

        bool is_running ();

        sigc::connection connect_signal_finish (sigc::slot<void> finish_slot);
        sigc::connection connect_signal_abort (sigc::slot<void> abort_slot);

    protected:
        // This is called on a separate thread
        Glib::RefPtr<Gio::Cancellable> cancellable ();

        virtual void run () = 0;

        virtual void on_finish () {}
        virtual void on_abort () {}

        class Cancelled;
        // Throws Cancelled if abort() was called
        void testcancelled ();

    private:
        struct Private;
        friend class Private;
        const std::unique_ptr<Private> _priv;
    };


    class AsyncOperation::Cancelled : public std::exception
    {
    public:
        Cancelled () {}
        virtual const char *what () const throw ();
    };
}

#endif  // _IMGPACK_ASYNC_OPERATION_HH
