#include <iostream>

#include <nihpp/singleton.hh>
#include <nihpp/sharedptrcreator.hh>

#include <imgpack/image-list.hh>
#include <imgpack/application.hh>
#include <imgpack/thread-pool.hh>

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

    Glib::RefPtr<Gdk::Pixbuf> load_pixbuf (const Glib::RefPtr<Gio::File> file,
                                           const int width, const int height)
    {
        LOG(info) << "Loading image from " << file->get_uri ();

        return Gdk::Pixbuf::create_from_stream_at_scale (file->read (),
                                                         width, height, true);
    }
}

using ImgPack::ImageList;


ImageList::ImageList (Application &app) :
    model (Gtk::ListStore::create (cols ())),
    app (app)
{
    set_model (model);

    set_pixbuf_column (cols ().thumbnail);
    set_text_column (cols ().filename);

    set_selection_mode (Gtk::SELECTION_MULTIPLE);

    set_reorderable (true);
    set_item_width (250);

    image_ready.connect (sigc::mem_fun (*this, &ImageList::on_image_ready));
}

void ImageList::add_image (const Glib::RefPtr<Gio::File> &file,
                           Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    if (!pixbuf)
        pixbuf = load_pixbuf (file, get_icon_width (), -1);

    Gtk::TreeIter iter = model->append ();
    iter->set_value (cols ().file, file);
    iter->set_value (cols ().uri, Glib::ustring (file->get_uri ()));
    iter->set_value (cols ().filename, Glib::ustring (file->get_basename ()));
    iter->set_value (cols ().thumbnail, pixbuf);

    LOG(info) << "Added image from " << file->get_uri ();
}

void ImageList::add_image_async (const Glib::RefPtr<Gio::File> &file)
{
    load_data_t data = {file,
                        ThreadPool::instance ().async (
                            std::bind (&load_pixbuf, file,
                                       get_icon_width (), -1),
                            image_ready)};

    load_queue.push_back (data);
}

void ImageList::remove_selected ()
{
    std::vector<Gtk::TreePath> paths = get_selected_items ();
    std::vector<Gtk::TreeIter> iters;

    iters.reserve (paths.size ());

    // This is needed because TreePaths get invalidated by removals
    for (Gtk::TreePath &i : paths)
        iters.push_back (model->get_iter (i));

    g_assert (paths.size () == iters.size ());

    for (Gtk::TreeIter &i : iters)
        model->erase (i);
}

void ImageList::on_image_ready ()
{
    while (!load_queue.empty () &&
           load_queue.front ().second.wait_for (std::chrono::nanoseconds (0))) {
        load_data_t data = load_queue.front ();
        load_queue.pop_front ();

        try {
            add_image (data.first, data.second.get ());

        } catch (Glib::Exception &e) {
            LOG(warning) << "Could not open image "
                         << data.first->get_uri ()
                         << ": Caught Glib::Exception ["
                         << e.what () << "]";
            // TODO: show error dialog
        }
    }
}