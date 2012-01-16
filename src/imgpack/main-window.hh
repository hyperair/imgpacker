#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    class StatusClient
    {
    public:
        typedef std::shared_ptr<StatusClient> Ptr;
        typedef std::weak_ptr<StatusClient> WPtr;

        virtual Gtk::Statusbar &statusbar () = 0;
        virtual Gtk::ProgressBar &progressbar () = 0;

        virtual bool live () = 0;

        virtual ~StatusClient () {}

    protected:
        StatusClient () {}
        StatusClient (const StatusClient &) = delete;
    };


    class StatusBusy : public std::exception
    {
    public:
        const char *what () const throw () {return "Statusbar in use";}
    };


    class Application;


    class MainWindow : public Gtk::Window
    {
    public:
        typedef std::shared_ptr<MainWindow> Ptr;
        typedef std::weak_ptr<MainWindow> WPtr;

        static Ptr create (Application &app);

        MainWindow () {}
        MainWindow (const MainWindow &) = delete;
        virtual ~MainWindow () {}

        virtual StatusClient::Ptr request_status () = 0;
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
