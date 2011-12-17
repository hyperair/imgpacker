#include <nihpp/singleton.hh>
#include <imgpack/image-list.hh>

namespace {
    class IconViewColumns : public Gtk::TreeModel::ColumnRecord,
                            public nihpp::Singleton<IconViewColumns>
    {
    public:
        Gtk::TreeModelColumn<Glib::RefPtr<Gio::File>>   file;
        Gtk::TreeModelColumn<Glib::ustring>             uri;
        Gtk::TreeModelColumn<Glib::ustring>             filename;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> thumbnail;

        IconViewColumns ()
        {
            add (file);
            add (uri);
            add (filename);
            add (thumbnail);
        }
    };

    // Get singleton instance of IconViewColumns
    inline IconViewColumns &cols ()
    {
        return IconViewColumns::instance ();
    }
}

using ImgPack::ImageList;

ImageList::ImageList () :
    model (Gtk::ListStore::create (cols ()))
{
    set_model (model);

    set_pixbuf_column (cols ().thumbnail);
    set_text_column (cols ().filename);
}

void ImageList::add_image (const Glib::RefPtr<Gio::File> &file,
                           Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    if (!pixbuf)
        pixbuf = Gdk::Pixbuf::create_from_stream_at_scale (file->read (),
                                                           250, -1, true);

    Gtk::TreeIter iter = model->append ();
    iter->set_value (cols ().file, file);
    iter->set_value (cols ().uri, Glib::ustring (file->get_uri ()));
    iter->set_value (cols ().filename, Glib::ustring (file->get_basename ()));
    iter->set_value (cols ().thumbnail, pixbuf);
}

void ImageList::add_image_async (const Glib::RefPtr<Gio::File> &file,
                                 SlotFinish finish_handler)
{
    // TODO: implement
}

void ImageList::remove_selected ()
{
    std::vector<Gtk::TreePath> paths = get_selected_items ();
    std::vector<Gtk::TreeIter> iters (paths.size ());

    // This is needed because TreePaths get invalidated by removals
    for (Gtk::TreePath &i : paths)
        iters.push_back (model->get_iter (i));

    for (Gtk::TreeIter &i : iters)
        model->erase (i);
}
