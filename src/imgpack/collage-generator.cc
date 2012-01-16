#include <imgpack/collage-generator.hh>

using namespace ImgPack;

namespace {
    class CollageGeneratorImpl :
        public CollageGenerator
    {
    public:
        virtual ~CollageGeneratorImpl () {}

        virtual void add_source (const Glib::RefPtr<Gdk::Pixbuf> &pixbuf);
        virtual void start ();
        virtual void abort ();

        virtual void connect_signal_finish (sigc::slot<void> slot);
        virtual void connect_signal_abort (sigc::slot<void> slot);

        virtual Rectangle::Ptr result () {return _result;}

    private:
        Rectangle::Ptr _result;
    };
}
