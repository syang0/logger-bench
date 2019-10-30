#include <stdio.h>

#include <bl/base/default_allocator.h>
#include <malc/malc.h>
#include <malc/destinations/file.h>

#include <malc.hpp>
#include <benchmark_iterables.hpp>

/*----------------------------------------------------------------------------*/
malc_base::malc_base()
{
    m_log     = nullptr;
    m_alloc   = get_default_alloc();
    m_created = false;
}
/*----------------------------------------------------------------------------*/
malc_base::~malc_base()
{
    destroy();
}
/*----------------------------------------------------------------------------*/
bool malc_base::create (std::size_t fixed_queues_bytes)
{
    bl_err err;
    m_log = (malc*) bl_alloc (&m_alloc, malc_get_size());
    if (!m_log) {
      fprintf (stderr, "Unable to allocate memory for the malc instance\n");
      return false;
    }
    err = malc_create (m_log, &m_alloc);
    if (err.bl) {
      fprintf (stderr, "Error creating the malc instance\n");
      destroy();
      return false;
    }
    m_created = true;
    /* destination register */
    u32 file_id;
    err = malc_add_destination (m_log, &file_id, &malc_file_dst_tbl);
    if (err.bl) {
      fprintf (stderr, "Error creating the file destination\n");
      destroy();
      return false;
    }
    /* logger startup */
    malc_cfg cfg;
    err = malc_get_cfg (m_log, &cfg);
    if (err.bl) {
      fprintf (stderr, "bug when retrieving the logger configuration\n");
      destroy();
      return false;
    }
    cfg.consumer.start_own_thread = true;

    cfg.alloc.fixed_allocator_bytes     = 0;
    cfg.alloc.fixed_allocator_max_slots = 0;
    cfg.alloc.fixed_allocator_per_cpu   = 0;
    cfg.producer.timestamp = true;

    set_cfg (cfg, fixed_queues_bytes);
    err = malc_init (m_log, &cfg);
    if (err.bl) {
        fprintf (stderr, "unable to start logger\n");
        destroy();
        return false;
    }
    return true;
}
/*----------------------------------------------------------------------------*/
bool malc_base::terminate()
{
    malc_terminate (m_log, false);
}
/*----------------------------------------------------------------------------*/
void malc_base::destroy()
{
    if (!m_log) {
        return;
    }
    if (m_created) {
        malc_destroy (m_log);
    }
    bl_dealloc (&m_alloc, m_log);
    m_log = nullptr;
}
/*----------------------------------------------------------------------------*/
template <class T>
std::size_t malc_base::run_logging (T& iterable)
{
    bl_err err;
    std::size_t success = 0;
    int i = 0;
    for (auto _ : iterable) {
        log_error (err, STRING_TO_LOG " {}", ++i);
        success += (err.bl == bl_ok);
    }
    return success;
}
INSTANTIATE_RUN_LOGGING_TEMPLATES (malc_base)
/*----------------------------------------------------------------------------*/
char const* malc_tls::get_name() const
{
    return "malc-tls";
}
/*----------------------------------------------------------------------------*/
char const* malc_tls::get_description() const
{
    return "mal C using thread local storage only";
}
/*----------------------------------------------------------------------------*/
bool malc_tls::prepare_thread(std::size_t fixed_queues_bytes)
{
    bl_err err = malc_producer_thread_local_init (m_log, fixed_queues_bytes);
    return  err.bl == bl_ok;
}
/*----------------------------------------------------------------------------*/
void malc_tls::set_cfg (struct malc_cfg& cfg, std::size_t fixed_queues_bytes)
{
    cfg.alloc.msg_allocator = nullptr;
}
/*----------------------------------------------------------------------------*/
char const* malc_heap::get_name() const
{
    return "malc-heap";
}
/*----------------------------------------------------------------------------*/
char const* malc_heap::get_description() const
{
    return "mal C using the default heap only";
}
/*----------------------------------------------------------------------------*/
void malc_heap::set_cfg (struct malc_cfg& cfg, std::size_t fixed_queues_bytes)
{}
/*----------------------------------------------------------------------------*/
char const* malc_fixed::get_name() const
{
    return "malc-fixed";
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed::get_description() const
{
    return "mal C using the fixed size memory pool";
}
/*----------------------------------------------------------------------------*/
void malc_fixed::set_cfg (struct malc_cfg& cfg, std::size_t fixed_queues_bytes)
{
    cfg.alloc.msg_allocator             = nullptr;
    cfg.alloc.fixed_allocator_bytes     = fixed_queues_bytes;
    cfg.alloc.fixed_allocator_max_slots = 2;
    cfg.alloc.fixed_allocator_per_cpu   = 0;
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_cpu::get_name() const
{
    return "malc-fixed-cpu";
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_cpu::get_description() const
{
    return "mal C using one fixed size memory pool for each CPU";
}
/*----------------------------------------------------------------------------*/
void malc_fixed_cpu::set_cfg(
    struct malc_cfg& cfg, std::size_t fixed_queues_bytes
    )
{
    cfg.alloc.msg_allocator         = nullptr;
    cfg.alloc.fixed_allocator_bytes = fixed_queues_bytes / bl_get_cpu_count();
    cfg.alloc.fixed_allocator_max_slots = 2;
    cfg.alloc.fixed_allocator_per_cpu   = 1;
}
/*----------------------------------------------------------------------------*/
void malc_tls_heap::set_cfg(
    struct malc_cfg& cfg, std::size_t fixed_queues_bytes
    )
{}
/*----------------------------------------------------------------------------*/
char const* malc_tls_heap::get_name() const
{
    return "malc-tls-heap";
}
/*----------------------------------------------------------------------------*/
char const* malc_tls_heap::get_description() const
{
    return "as malc-tls but using the as heap fallback";
}
/*----------------------------------------------------------------------------*/
void malc_fixed_heap::set_cfg(
    struct malc_cfg& cfg, std::size_t fixed_queues_bytes
    )
{
    auto alloc = cfg.alloc.msg_allocator;
    malc_fixed::set_cfg (cfg, fixed_queues_bytes);
    cfg.alloc.msg_allocator = alloc;
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_heap::get_name() const
{
    return "malc-fixed-heap";
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_heap::get_description() const
{
    return "as malc-fixed but using the as heap fallback";
}
/*----------------------------------------------------------------------------*/
void malc_fixed_cpu_heap::set_cfg(
    struct malc_cfg& cfg, std::size_t fixed_queues_bytes
    )
{
    auto alloc = cfg.alloc.msg_allocator;
    malc_fixed_cpu::set_cfg (cfg, fixed_queues_bytes);
    cfg.alloc.msg_allocator = alloc;
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_cpu_heap::get_name() const
{
    return "malc-fixed-cpu-heap";
}
/*----------------------------------------------------------------------------*/
char const* malc_fixed_cpu_heap::get_description() const
{
    return "as malc-fixed-cpu but using the as heap fallback";
}
/*----------------------------------------------------------------------------*/
