#ifndef IMGPACK_BINPACKER_HH
#define IMGPACK_BINPACKER_HH

#include <memory>
#include <vector>

#include <imgpack/util/async-operation.hh>
#include <imgpack/algorithm/rectangles.hh>

namespace ImgPack
{
    namespace Algorithm
    {
        class BinPacker : virtual public Util::AsyncOperation
        {
        public:
            typedef std::shared_ptr<BinPacker> Ptr;
            typedef std::list<Rectangle::Ptr> RectangleList;

            static Ptr create ();
            virtual ~BinPacker () {}

            virtual void target_aspect (double aspect_ratio) = 0;
            virtual void source_rectangles (RectangleList rectangles) = 0;

            virtual Rectangle::Ptr result () = 0;

        protected:
            BinPacker () {}
        };
    }
}

#endif  // IMGPACK_BINPACKER_HH
