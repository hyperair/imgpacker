#ifndef IMGPACK_GTKUI_GTK_APPLICATION_HH
#define IMGPACK_GTKUI_GTK_APPLICATION_HH

#include <gtkmm.h>

#include <imgpack/application.hh>

namespace ImgPack
{
    namespace GtkUI
    {
        class GtkApplication : public ImgPack::Application,
                               public Gtk::Main
        {
        public:
            GtkApplication (int &argc, char **&argv);
            virtual ~GtkApplication ();

            virtual void run ();
            virtual void show_about ();
            virtual void spawn_window ();

        private:
            class Private;
            std::unique_ptr<Private> _priv;
        };
    }
}

#endif  // IMGPACK_GTKUI_GTK_APPLICATION_HH
