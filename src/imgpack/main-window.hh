#ifndef IMGPACK_MAIN_WINDOW_HH
#define IMGPACK_MAIN_WINDOW_HH

#include <gtkmm.h>
#include <nihpp/sharedptrcreator.hh>
#include <imgpack/image-list.hh>

namespace ImgPack
{
    class Application;

    class MainWindow : public Gtk::Window
    {
    public:
        explicit MainWindow (Application &app);
        MainWindow (const MainWindow &) = delete;
        ~MainWindow ();

        Gtk::Statusbar &statusbar () {return _statusbar;}
        Gtk::ProgressBar &progressbar () {return _progressbar;}


        class Operation;
        class OperationActive;

        void set_operation (const std::shared_ptr<Operation> &operation);
        void unset_operation ();

    private:
        Application                 &app;
        Glib::RefPtr<Gtk::UIManager> uimgr;
        void                         init_uimgr ();

        Gtk::VBox                    main_vbox;
        Gtk::HPaned                  main_pane;

        ImageList                    image_list;
        Gtk::DrawingArea             preview;

        Gtk::Statusbar               _statusbar;
        Gtk::ProgressBar             _progressbar;

        std::shared_ptr<Operation>   operation;
        sigc::connection             operation_finish_connection;

        class PixbufLoader;
        std::shared_ptr<PixbufLoader> pixbuf_loader;
        // callbacks
        void on_add ();
        void on_exec ();
    };


    /**
     * Abstract base class for long-running operations that require touching the
     * statusbar and progressbar of the MainWindow.
     */
    class MainWindow::Operation : public nihpp::SharedPtrCreator<Operation>
    {
    public:
        Operation () {}
        Operation (const Operation &) = delete;

        /**
         * Attaches Operation to mainwindow and starts the operation
         */
        virtual void attach (MainWindow &mainwindow) = 0;

        /**
         * Detaches the Operation from the mainwindow and stops the operation
         */
        virtual void detach () = 0;

        sigc::connection
        connect_signal_finish (sigc::slot<void> finish_slot)
        {
            return finish_signal.connect (finish_slot);
        }

    protected:
        void finish () {finish_signal.emit ();}

    private:
        sigc::signal<void> finish_signal;
    };

    /**
     * Exception that is thrown by MainWindow::set_operation() if an operation
     * is still active.
     */
    class MainWindow::OperationActive : public std::exception
    {
    public:
        OperationActive () = delete;
        OperationActive (const OperationActive &) = default;

        OperationActive (const Operation::Ptr &current) :
            _current (current) {}

        virtual ~OperationActive () throw () {}

        virtual const char *what () const throw ()
        {
            return "Operation still active";
        }

        /**
         * Get currently active operation
         * @returns operation that was active when the exception was thrown
         */
        Operation::Ptr current () {return _current;}

    private:
        Operation::Ptr _current;
    };
}

#endif  // IMGPACK_MAIN_WINDOW_HH
