#include <unistd.h>
#include <imgpack/thread-pool.hh>
#include <imgpack/logger.hh>

using ImgPack::ThreadPool;

ThreadPool::ThreadPool () :
    Glib::ThreadPool (sysconf (_SC_NPROCESSORS_ONLN))
{
    LOG(info) << "Initialized thread pool with max threads: "
              << get_max_threads ();
}

ThreadPool::~ThreadPool () {}
