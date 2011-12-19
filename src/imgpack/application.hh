#ifndef IMGPACK_IMGPACK_APPLICATION_HH
#define IMGPACK_IMGPACK_APPLICATION_HH

#include <future>
#include <memory>

#include <gtkmm.h>
#include <imgpack/main-window.hh>

namespace ImgPack
{
    class Application : public ::Gtk::Main
    {
    public:
        Application (int &argc, char **&argv);
        virtual ~Application () {}

        void run ();            // Hide Gtk::Main::run

        void show_about ();

        Glib::ThreadPool &thread_pool () {return thread_pool_;}

        // Reimplemented std::async which takes a callable (with no arguments,
        // so bind() or a lambda should be used to pass arguments), and a
        // dispatcher which is called when the value in the std::future is ready
        template <typename T>
        std::future<typename std::result_of<T ()>::type>
            async_task (T callable, Glib::Dispatcher &finish_signal)
        {
            typedef typename std::result_of<T ()>::type ret;
            std::shared_ptr<std::promise<ret>> promise (new std::promise<ret>);

            thread_pool ().push ([=, &finish_signal]() {
                    try {
                        promise->set_value (std::move (callable ()));

                    } catch (...) {
                        promise->set_exception(std::current_exception ());
                    }

                    finish_signal ();
                });

            return promise->get_future ();
        }

    private:
        MainWindow       main_window;
        Gtk::AboutDialog about_dialog;
        Glib::ThreadPool thread_pool_;
    };
}

#endif  // IMGPACK_APPLICATION_HH
