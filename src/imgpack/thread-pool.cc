#include <unistd.h>
#include <imgpack/thread-pool.hh>
#include <imgpack/util/logger.hh>

using ImgPack::ThreadPool;

ThreadPool::ThreadPool () :
    Glib::ThreadPool (hardware_concurrency ())
{
    LOG(info) << "Initialized thread pool with max threads: "
              << get_max_threads ();
}

ThreadPool::~ThreadPool (){}

// static
long ThreadPool::hardware_concurrency ()
{
    return sysconf (_SC_NPROCESSORS_ONLN);
}
