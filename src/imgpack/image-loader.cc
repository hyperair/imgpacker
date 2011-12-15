#include <imgpack/image-loader.hh>

using ImgPack::ImageLoader;

ImageLoader::ImageLoader ()
{
    init ();
}

ImageLoader::ImageLoader (const SlotFinish &finish_handler)
{
    init ();
    signal_finish_.connect (finish_handler);
}

void ImageLoader::init ()
{
    dispatcher.connect (sigc::mem_fun (*this, &ImageLoader::load_finish));
    thread = nullptr;
}

ImageLoader::~ImageLoader ()
{
    // TODO: clean the thread up
}

void ImageLoader::push (const Glib::RefPtr<Gio::File> file)
{
    Glib::Mutex::Lock l (mutex);

    unlocked_push (file);
}

void ImageLoader::unlocked_push (const Glib::RefPtr<Gio::File> file)
{
    input_queue.push (file);

    if (!thread)
        thread = Glib::Thread::create (sigc::mem_fun
                                       (*this, &ImageLoader::load_images),
                                       false);
}

void ImageLoader::load_images ()
{
    Glib::RefPtr<Gio::File> current_file;

    for (;;) {
        {
            Glib::Mutex::Lock l (mutex);

            if (input_queue.empty ()) {
                thread = nullptr;
                dispatcher ();
                return;
            }

            current_file = input_queue.front ();
            input_queue.pop ();
        }

        Glib::RefPtr<Gdk::Pixbuf> pixbuf =
            Gdk::Pixbuf::create_from_stream_at_scale (current_file->read (),
                                                      200, -1, true);

        {
            Glib::Mutex::Lock l (mutex);

            output_queue.push ({current_file, pixbuf});
        }
    }
}

void ImageLoader::load_finish ()
{
    Glib::Mutex::Lock l (mutex);
    auto queue = std::move (output_queue);
    l.release ();

    while (!queue.empty ()) {
        auto pair = queue.front ();
        queue.pop ();

        signal_finish_ (pair.first, pair.second);
    }
}
