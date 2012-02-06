#include <unordered_set>
#include <glibmm/i18n.h>

#include <imgpack/util/logger.hh>
#include <imgpack/gtkui/gtk-application.hh>
#include <imgpack/gtkui/main-window.hh>
#include <config.h>

namespace ip = ImgPack;
namespace ipg = ImgPack::GtkUI;

using ipg::GtkApplication;

struct GtkApplication::Private
{
    std::unordered_set<std::shared_ptr<MainWindow> > windows;
    Gtk::AboutDialog about_dialog;
};

GtkApplication::GtkApplication (int &argc, char **&argv) :
    Gtk::Main (argc, argv),
    _priv (new Private)
{
    Glib::set_application_name (_("ImgPacker Collage Creator"));

    // initialize about_dialog
    _priv->about_dialog.set_version (VERSION);
    _priv->about_dialog.set_license_type (Gtk::LICENSE_GPL_3_0);
    _priv->about_dialog.set_authors ({"Chow Loong Jin <hyperair@ubuntu.com>"});
    _priv->about_dialog.signal_response ()
        .connect ([&](int) {_priv->about_dialog.hide ();});
}

GtkApplication::~GtkApplication () {}

void GtkApplication::run ()
{
    spawn_window ();
    Gtk::Main::run ();
}

void GtkApplication::show_about ()
{
    _priv->about_dialog.show ();
}

void GtkApplication::spawn_window ()
{
    std::shared_ptr<MainWindow> window (new MainWindow (*this));
    std::weak_ptr<MainWindow> weak_window = window; // for lambda

    window->signal_hide ().connect ([=] () {
            LOG(info) << "A window was closed. Removing from window list.";

            _priv->windows.erase (weak_window.lock ());

            if (_priv->windows.empty ()) {
                LOG(info) << "No remaining windows. Quitting.";
                quit ();

            } else
                LOG(info) << "Remaining open windows: "
                          << _priv->windows.size ();
        });

    _priv->windows.insert (window);
    window->show ();
}
