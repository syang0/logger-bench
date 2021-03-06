#include <stdio.h>

#include <nanolog.hpp>
#include <NanoLogCpp17.h>

#include <benchmark_iterables.hpp>

static const char logfile[] = "./logger-bench-nanolog";

/*----------------------------------------------------------------------------*/
bool nanolog::create (std::size_t fixed_queues_bytes)
{
    NanoLog::setLogFile (logfile);
    NanoLog::setLogLevel (NanoLog::DEBUG);
    return true;
}
/*----------------------------------------------------------------------------*/
bool nanolog::terminate()
{
    NanoLog::sync();
    remove (logfile);
    return true;
}
/*----------------------------------------------------------------------------*/
void nanolog::destroy() {}
/*----------------------------------------------------------------------------*/
bool nanolog::prepare_thread(std::size_t fixed_queues_bytes)
{
    return true;
}
/*----------------------------------------------------------------------------*/
template <class T>
std::size_t nanolog::run_logging (T& iterable)
{
    std::size_t success = 0;
    int i = 0;
    for (auto _ : iterable) {
        NANO_LOG (NanoLog::NOTICE, "%s %i", STRING_TO_LOG, ++i);
        ++success;
    }
    return success;
}
INSTANTIATE_RUN_LOGGING_TEMPLATES (nanolog)
/*----------------------------------------------------------------------------*/
char const* nanolog::get_name() const
{
    return "nanolog";
}
/*----------------------------------------------------------------------------*/
char const* nanolog::get_description() const
{
    return "NanoLog C++17 ((C) Standford University)";
}
/*----------------------------------------------------------------------------*/
bool nanolog_tls::prepare_thread(std::size_t fixed_queues_bytes)
{
    /*NOTICE, no way to pass memory size restructions*/
    NanoLog::preallocate();
    return true;
}
/*----------------------------------------------------------------------------*/
char const* nanolog_tls::get_name() const
{
    return "nanolog-tls";
}
/*----------------------------------------------------------------------------*/
char const* nanolog_tls::get_description() const
{
    return "NanoLog C++17 ((C) Standford University calling NanoLog::preallocate()";
}
/*----------------------------------------------------------------------------*/
