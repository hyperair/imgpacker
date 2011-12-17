#include <nihpp/singleton.hh>
#include <nihpp/sharedptrcreator.hh>

#include <imgpack/image-list.hh>
#include <imgpack/application.hh>

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

    Glib::RefPtr<Gdk::Pixbuf> load_pixbuf (const Glib::RefPtr<Gio::File> file)
    {
        return Gdk::Pixbuf::create_from_stream_at_scale (file->read (),
                                                         250, -1, true);
    }

    template <typename Ptr>
    class TaskWrapper
    {
    public:
        TaskWrapper (const Ptr &ptr) : ptr (ptr) {}

        template <typename... Args>
        auto operator() (Args&&... args)
            -> decltype ((*Ptr ()) (std::forward<Args> (args)...))
        {
            return (*ptr) (std::forward<Args> (args)...);
        }

    private:
        Ptr ptr;
    };
}

using ImgPack::ImageList;


struct ImageList::LoaderTask :
    public nihpp::SharedPtrCreator<ImageList::LoaderTask>
{
    LoaderTask (const Glib::RefPtr<Gio::File> &file,
                Glib::Dispatcher &finish);
    void operator() ();         // Run by threadpool

    explicit operator bool ();

    Glib::RefPtr<Gio::File> file;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    Glib::Dispatcher &finish;
};

ImageList::LoaderTask::LoaderTask (const Glib::RefPtr<Gio::File> &file,
                                   Glib::Dispatcher &finish) :
    file (file),
    pixbuf (nullptr),

    finish (finish)
{
}

void ImageList::LoaderTask::operator() ()
{
    pixbuf = load_pixbuf (file);
    finish ();
}

ImageList::LoaderTask::operator bool ()
{
    return pixbuf;
}


ImageList::ImageList (Application &app) :
    model (Gtk::ListStore::create (cols ())),
    app (app)
{
    set_model (model);

    set_pixbuf_column (cols ().thumbnail);
    set_text_column (cols ().filename);

    image_ready.connect (sigc::mem_fun (*this, &ImageList::on_image_ready));
}

void ImageList::add_image (const Glib::RefPtr<Gio::File> &file,
                           Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    if (!pixbuf)
        pixbuf = load_pixbuf (file);

    Gtk::TreeIter iter = model->append ();
    iter->set_value (cols ().file, file);
    iter->set_value (cols ().uri, Glib::ustring (file->get_uri ()));
    iter->set_value (cols ().filename, Glib::ustring (file->get_basename ()));
    iter->set_value (cols ().thumbnail, pixbuf);
}

void ImageList::add_image_async (const Glib::RefPtr<Gio::File> &file)
{
    LoaderTask::Ptr loader_task = LoaderTask::create (file, image_ready);

    load_queue.push_back (loader_task);
    app.thread_pool ().push (TaskWrapper<LoaderTask::Ptr> (loader_task));
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
    while (!load_queue.empty () && *load_queue.front ()) {
        LoaderTask::Ptr task = load_queue.front ();
        add_image (task->file, task->pixbuf);

        load_queue.pop_front ();
    }
}
