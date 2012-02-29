#ifndef IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH
#define IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH

#include <memory>
#include <gtkmm.h>

#include <imgpack/algorithm/rectangles.hh>

namespace ImgPack
{
    namespace GtkUI
    {
        class CollageTreeView : public Gtk::TreeView
        {
            CollageTreeView (const CollageTreeView &) = delete;

        public:
            CollageTreeView ();
            virtual ~CollageTreeView ();

            void collage (Algorithm::Rectangle::Ptr rect);

        private:
            class Private;
            std::unique_ptr<Private> _priv;
        };
    }
}

#endif  // IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH
