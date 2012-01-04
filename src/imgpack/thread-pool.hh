#ifndef IMGPACK_THREAD_POOL_HH
#define IMGPACK_THREAD_POOL_HH

#include <future>
#include <glibmm.h>
#include <nihpp/singleton.hh>

namespace ImgPack
{
    class ThreadPool : public Glib::ThreadPool,
                       public nihpp::Singleton<ThreadPool>
    {
    public:
        ThreadPool ();
        ~ThreadPool ();

        // Reimplemented std::async which takes a callable (with no arguments,
        // so bind() or a lambda should be used to pass arguments), and a
        // dispatcher which is called when the value in the std::future is ready
        template <typename T>
        std::future<typename std::result_of<T()>::type>
        async (T callable, Glib::Dispatcher &finish_signal);
    };


    // Template Definitions
    template <typename T>
    std::future<typename std::result_of<T ()>::type>
    ThreadPool::async (T callable, Glib::Dispatcher &finish_signal)
    {
        typedef typename std::result_of<T ()>::type ret;
        std::shared_ptr<std::promise<ret> > promise (new std::promise<ret>);

        push ([=, &finish_signal]() {
                try {
                    promise->set_value (std::move (callable ()));

                } catch (...) {
                    promise->set_exception(std::current_exception ());
                }

                finish_signal ();
            });

        return promise->get_future ();
    }
}

#endif  // IMGPACK_THREAD_POOL_HH
