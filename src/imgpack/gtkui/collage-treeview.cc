#include <imgpack/gtkui/collage-treeview.hh>
#include <imgpack/util/logger.hh>
#include <imgpack/algorithm/rectangles.hh>
#include <nihpp/glib/refptrcreator.hh>

namespace ip = ImgPack;
namespace ipg = ip::GtkUI;
namespace ipa = ip::Algorithm;

using ipg::CollageTreeView;

namespace {
    class RectangleTreeModel :
        public Glib::Object,
        public Gtk::TreeModel,
        public nihpp::Glib::RefPtrCreator<RectangleTreeModel>
    {
    public:
        RectangleTreeModel ();
        ~RectangleTreeModel ();

        void collage (ipa::Rectangle::Ptr root);

    private:
        // Implementation of Gtk::TreeModel
        virtual Gtk::TreeModelFlags get_flags_vfunc () const;

        virtual bool get_iter_vfunc (const Path &, iterator &) const;

        virtual int get_n_columns_vfunc () const;

        virtual GType get_column_type_vfunc (int index) const;

        virtual bool iter_next_vfunc (const iterator &iter,
                                      iterator &iter_next) const;

        virtual bool iter_children_vfunc (const iterator &parent,
                                          iterator &iter) const;

        virtual bool iter_parent_vfunc  (const iterator &child,
                                         iterator &iter) const;

        virtual bool iter_nth_child_vfunc (const iterator &parent, int n,
                                           iterator &iter) const;

        virtual bool iter_has_child_vfunc (const iterator &iter) const;

        virtual int iter_n_children_vfunc (const iterator &iter) const;

        virtual int iter_n_root_children_vfunc () const;

        virtual Path get_path_vfunc (const iterator &iter) const;

        virtual void get_value_vfunc (const iterator &iter, int column,
                                      Glib::ValueBase &value) const;

        ipa::Rectangle::Ptr collage_root;
    };

    ipa::Rectangle::Ptr iter_to_rect (const RectangleTreeModel::iterator & iter)
    {
        const GtkTreeIter *iter_gobj = iter->gobj ();

        ipa::Rectangle *rect =
            static_cast<ipa::Rectangle *> (iter_gobj->user_data);

        g_assert (rect);

        return rect->shared_from_this ();
    }

    bool rect_to_iter (const ipa::Rectangle::Ptr rect,
                       RectangleTreeModel::iterator &iter)
    {
        if (!rect)
            return false;

        GtkTreeIter *rawiter = iter.gobj ();
        rawiter->user_data = rect.get ();

        return true;
    }
}


RectangleTreeModel::RectangleTreeModel () :
    Glib::ObjectBase (typeid (RectangleTreeModel))
{
}

RectangleTreeModel::~RectangleTreeModel ()
{
}

void RectangleTreeModel::collage (ipa::Rectangle::Ptr root)
{
    if (collage_root) {
        iterator iter;

        g_assert (rect_to_iter (collage_root, iter));
        row_deleted (get_path_vfunc (iter));

        collage_root.reset ();
    }

    g_assert (root);
    collage_root = std::move (root);

    iterator iter;
    g_assert (rect_to_iter (collage_root, iter));

    Path path = get_path_vfunc (iter);
    row_inserted (path, iter);
}

Gtk::TreeModelFlags RectangleTreeModel::get_flags_vfunc () const
{
    return Gtk::TREE_MODEL_ITERS_PERSIST;
}

bool RectangleTreeModel::get_iter_vfunc (const Path &path, iterator &iter) const

{
    ipa::Rectangle::Ptr rect;

    bool visited_collage_root = false;

    for (int i : path) {
        if (!rect)
            if (collage_root && !visited_collage_root) {
                g_assert (i == 0);
                rect = collage_root;
                visited_collage_root = true;

            } else
                return false;

        else
            switch (i) {
            case 0:
                rect = rect->child1 ();
                break;

            case 1:
                rect = rect->child2 ();
                break;

            default:
                return false;
            }
    }

    return rect_to_iter (rect, iter);
}

int RectangleTreeModel::get_n_columns_vfunc () const
{
    return 1;
}

GType RectangleTreeModel::get_column_type_vfunc (int index) const
{
    // We only show a string.
    g_assert (index == 0);
    return G_TYPE_STRING;
}

bool RectangleTreeModel::iter_next_vfunc (const iterator &iter,
                                          iterator &iter_next) const
{
    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);
    const ipa::Rectangle::Ptr parent = rect->parent ();

    if (!parent)
        return false;

    if (parent->child1 () == rect)
        return rect_to_iter (parent->child2 (), iter_next);

    else
        g_assert (parent->child2 () == rect);

    return false;
}

bool RectangleTreeModel::iter_children_vfunc (const iterator &iter,
                                              iterator &iter_child) const
{
    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);
    const ipa::Rectangle::Ptr child = rect->child1 ();

    return rect_to_iter (child, iter_child);
}

bool RectangleTreeModel::iter_parent_vfunc  (const iterator &iter,
                                             iterator &iter_parent) const
{
    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);
    const ipa::Rectangle::Ptr parent = rect->parent ();

    return rect_to_iter (parent, iter_parent);
}

bool RectangleTreeModel::iter_nth_child_vfunc (const iterator &iter,
                                               int n,
                                               iterator &iter_child) const
{
    if (n < 0 || n > 1)
        return false;

    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);
    const ipa::Rectangle::Ptr child =
        (n == 0) ? rect->child1 () : rect->child2 ();

    return rect_to_iter (child, iter_child);
}

bool RectangleTreeModel::iter_has_child_vfunc (const iterator &iter) const
{
    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);

    return bool (rect->child1 ());
}

int RectangleTreeModel::iter_n_children_vfunc (const iterator &iter) const
{
    return iter_has_child_vfunc (iter) ? 2 : 0;
}

int RectangleTreeModel::iter_n_root_children_vfunc () const
{
    return collage_root ? 1 : 0;
}

RectangleTreeModel::Path
RectangleTreeModel::get_path_vfunc (const iterator &iter) const
{
    Path path;

    ipa::Rectangle::Ptr rect = iter_to_rect (iter);
    g_assert (rect);

    ipa::Rectangle::Ptr parent;

    while ((parent = rect->parent ()) && parent != collage_root) {
        int offset;

        if (parent->child1 () == rect)
            offset = 0;

        else if (parent->child2 () == rect)
            offset = 1;

        else
            g_assert_not_reached ();

        path.push_front (offset);

        rect = parent;
    }

    path.push_front (0);        // for root node

    return path;
}

void RectangleTreeModel::get_value_vfunc (const iterator &iter, int col,
                                          Glib::ValueBase &value) const
{
    g_assert (col == 0);

    const ipa::Rectangle::Ptr rect = iter_to_rect (iter);

    Glib::Value<Glib::ustring> str_value;
    str_value.init (Glib::Value<Glib::ustring>::value_type ());
    str_value.set (rect->description ());

    value.init (Glib::Value<Glib::ustring>::value_type ());
    value = str_value;
}


struct CollageTreeView::Private
{
    Private ();

    RectangleTreeModel::Ptr model;
};

CollageTreeView::Private::Private () :
    model (RectangleTreeModel::create ())
{}

CollageTreeView::CollageTreeView () :
    _priv (new Private)
{
    set_model (_priv->model);
}

CollageTreeView::~CollageTreeView () {}

void CollageTreeView::collage (ipa::Rectangle::Ptr collage)
{
    LOG(info) << "Updating collage";
    _priv->model->collage (std::move (collage));
}
