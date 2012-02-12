#ifndef IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH
#define IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH

#include <gtkmm.h>

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
        };
    }
}

#endif  // IMGPACK_GTKUI_COLLAGE_TREEVIEW_HH
