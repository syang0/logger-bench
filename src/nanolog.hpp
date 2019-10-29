#ifndef __BENCHMARK_NANOLOG__
#define __BENCHMARK_NANOLOG__

#include <logger_impl_helpers.hpp>
/*----------------------------------------------------------------------------*/
class nanolog : public logger_adaptor<nanolog> {
public:
    virtual ~nanolog() {};
    virtual char const* get_name() const;
    virtual char const* get_description() const;
    virtual bool create (int fixed_queues_bytes);
    virtual void destroy();
    virtual bool terminate();
    virtual bool prepare_thread(int fixed_queues_bytes);
    template <class T>
    int run_logging (T& iterable);
};
/*----------------------------------------------------------------------------*/
#endif