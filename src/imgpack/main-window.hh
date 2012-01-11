#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <nihpp/sharedptrcreator.hh>
#include <gtkmm.h>
#include <imgpack/image-list.hh>

namespace ImgPack
{
    class Application;
    class MainWindow;
    class StatusController;

    class StatusClient :
        private nihpp::SharedPtrCreator<StatusClient>
    {
        friend class MainWindow;
        friend class StatusController;
        friend class nihpp::SharedPtrCreator<StatusClient>;
        friend class nihpp::PtrCreator<StatusClient, Ptr>;

    public:
        using nihpp::SharedPtrCreator<StatusClient>::Ptr;
        using nihpp::SharedPtrCreator<StatusClient>::WPtr;

        Gtk::Statusbar &statusbar ();
        Gtk::ProgressBar &progressbar ();

        bool live () {return controller;}

    private:
        explicit StatusClient (StatusController &controller) :
            controller (&controller) {}

        void unlink ();

        StatusController *controller;
    };


    class StatusController
    {
        friend class StatusClient;
        friend class MainWindow;

    public:
        StatusController ();
        StatusController (const StatusController &) = delete;
        ~StatusController ();

        StatusClient::Ptr request ();

    private:
        Gtk::Statusbar statusbar;
        Gtk::ProgressBar progressbar;

        StatusClient::WPtr client;
    };


    class StatusBusy : public std::exception
    {
    public:
        const char *what () const throw () {return "Statusbar in use";}
    };


    class MainWindow : public Gtk::Window
    {
    public:
        explicit MainWindow (Application &app);
        MainWindow (const MainWindow &) = delete;
        ~MainWindow ();

        StatusClient::Ptr request_status () {return status.request ();}

    private:
        Application                 &app;
        Glib::RefPtr<Gtk::UIManager> uimgr;
        void                         init_uimgr ();

        Gtk::VBox                    main_vbox;
        Gtk::HPaned                  main_pane;

        ImageList                    image_list;
        Gtk::DrawingArea             preview;

        StatusController             status;
        Gtk::Statusbar &statusbar () {return status.statusbar;}
        Gtk::ProgressBar &progressbar () {return status.progressbar;}

        // callbacks
        void on_add ();
        void on_exec ();
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
