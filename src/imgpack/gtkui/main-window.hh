#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <memory>
#include <gtkmm.h>

namespace ImgPack
{
    class Application;

    namespace GtkUI
    {
        class StatusController;

        class StatusClient
        {
            friend class StatusController;

        public:
            ~StatusClient ();

            typedef std::shared_ptr<StatusClient> Ptr;
            typedef std::weak_ptr<StatusClient> WPtr;

            Gtk::Statusbar     &statusbar ();
            Gtk::ProgressBar   &progressbar ();

            bool is_live ();

        private:
            StatusClient (StatusController &controller);
            StatusClient (const StatusClient &) = delete;

            StatusController *controller;
            void unlink () {controller = nullptr;}
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
            ~MainWindow ();

            StatusClient::Ptr request_status ();

        private:
            class Private;
            std::unique_ptr<Private> _priv;
        };
    }
}

#endif  // IMGPACK_MAIN_WINDOW_HH
