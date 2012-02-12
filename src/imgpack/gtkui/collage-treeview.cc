#include <imgpack/gtkui/collage-treeview.hh>
#include <nihpp/glib/refptrcreator.hh>

namespace ip = ImgPack;
namespace ipg = ImgPack::GtkUI;

using ipg::CollageTreeView;

namespace {
    class RectangleTreeModel :
        public Gtk::TreeModel,
        public nihpp::Glib::RefPtrCreator<RectangleTreeModel>
    {
    public:
        RectangleTreeModel ();
        ~RectangleTreeModel ();

    private:
        // Implementation of Gtk::TreeModel
        virtual Gtk::TreeModelFlags get_flags_vfunc () const;

        virtual int get_n_columns_vfunc () const;

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

        virtual void set_value_impl (const iterator &row, int column,
                                     const Glib::ValueBase &value);

        virtual void get_value_impl (const iterator &row, int column,
                                     Glib::ValueBase &value) const;
    };
}

RectangleTreeModel::RectangleTreeModel () :
    Glib::ObjectBase (typeid (*this))
{
}

RectangleTreeModel::~RectangleTreeModel ()
{
}

Gtk::TreeModelFlags RectangleTreeModel::get_flags_vfunc () const
{
    return Gtk::TREE_MODEL_ITERS_PERSIST;
}

int RectangleTreeModel::get_n_columns_vfunc () const
{
    return 1;
}

bool RectangleTreeModel::iter_next_vfunc (const iterator &,
                                          iterator &) const
{
    return false;
}

bool RectangleTreeModel::iter_children_vfunc (const iterator &,
                                              iterator &) const
{
    return false;
}

bool RectangleTreeModel::iter_parent_vfunc  (const iterator &,
                                             iterator &) const
{
    return false;
}

bool RectangleTreeModel::iter_nth_child_vfunc (const iterator &, int,
                                               iterator &) const
{
    return false;
}

bool RectangleTreeModel::iter_has_child_vfunc (const iterator &) const
{
    return false;
}

int RectangleTreeModel::iter_n_children_vfunc (const iterator &) const
{
    return 0;
}

int RectangleTreeModel::iter_n_root_children_vfunc () const
{
    return 0;
}

RectangleTreeModel::Path RectangleTreeModel::get_path_vfunc (const iterator &)
    const
{
    return Path ();
}

void RectangleTreeModel::get_value_vfunc (const iterator &, int,
                                          Glib::ValueBase &) const
{}

void RectangleTreeModel::set_value_impl (const iterator &, int,
                                         const Glib::ValueBase &)
{}

void RectangleTreeModel::get_value_impl (const iterator &, int,
                                         Glib::ValueBase &) const
{}


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



