#include "runtime_meter.h"
#include "runtime_meter_stats.h"

#include <limits.h>
#include <stddef.h>

#include "runtime_meter_port.h"

#if defined(__GNUC__) && !defined(__clang__)
#define RUNTIME_METER_NOINLINE __attribute__((noinline, noclone))
#elif defined(__clang__)
#define RUNTIME_METER_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define RUNTIME_METER_NOINLINE __declspec(noinline)
#else
#define RUNTIME_METER_NOINLINE
#endif

static uint32_t s_frequency_hz;
static runtime_meter_tick_t s_overhead_ticks;

/* The quotient fits in 32 bits at every component call site. */
static uint32_t divide_u64_by_u32(uint64_t numerator, uint32_t denominator)
{
    uint64_t multiple;
    uint32_t bit = 0x80000000UL;
    uint32_t quotient = 0U;

    if (denominator == 0U) {
        return 0U;
    }

    multiple = (uint64_t)denominator << 31U;
    while (bit != 0U) {
        if (numerator >= multiple) {
            numerator -= multiple;
            quotient |= bit;
        }
        multiple >>= 1U;
        bit >>= 1U;
    }

    return quotient;
}

bool runtime_meter_init(void)
{
    runtime_meter_tick_t best = UINT32_MAX;
    uint32_t frequency_hz = 0U;
    uint32_t i;

    /* Fail closed: stale calibration must not survive a failed re-init. */
    s_frequency_hz = 0U;
    s_overhead_ticks = 0U;

    if (!runtime_meter_port_init(&frequency_hz) || frequency_hz == 0U) {
        return false;
    }

    for (i = 0U; i < RUNTIME_METER_CFG_CALIBRATION_SAMPLES; ++i) {
        runtime_meter_tick_t start = runtime_meter_start();
        runtime_meter_tick_t elapsed = runtime_meter_stop(start);

        if (elapsed < best) {
            best = elapsed;
        }
    }

    s_frequency_hz = frequency_hz;
    s_overhead_ticks = best;
    return true;
}

RUNTIME_METER_NOINLINE runtime_meter_tick_t runtime_meter_start(void)
{
    return runtime_meter_port_now();
}

RUNTIME_METER_NOINLINE runtime_meter_tick_t
runtime_meter_stop(runtime_meter_tick_t start)
{
    runtime_meter_tick_t elapsed = runtime_meter_port_now() - start;

    return elapsed > s_overhead_ticks ? elapsed - s_overhead_ticks : 0U;
}

uint64_t runtime_meter_ticks_to_ns(runtime_meter_tick_t ticks)
{
    uint32_t whole_seconds;
    uint32_t remainder_ticks;
    uint32_t fractional_ns;

    if (s_frequency_hz == 0U) {
        return 0U;
    }

    whole_seconds = ticks / s_frequency_hz;
    remainder_ticks = ticks % s_frequency_hz;
    fractional_ns = divide_u64_by_u32(
        (uint64_t)remainder_ticks * 1000000000ULL + (s_frequency_hz / 2U),
        s_frequency_hz);

    if (fractional_ns >= 1000000000U) {
        ++whole_seconds;
        fractional_ns -= 1000000000U;
    }

    return (uint64_t)whole_seconds * 1000000000ULL + fractional_ns;
}

uint32_t runtime_meter_frequency_hz(void)
{
    return s_frequency_hz;
}

runtime_meter_tick_t runtime_meter_overhead_ticks(void)
{
    return s_overhead_ticks;
}

void runtime_meter_stats_reset(runtime_meter_stats_t *stats)
{
    if (stats == NULL) {
        return;
    }

    stats->count = 0U;
    stats->min_ticks = UINT32_MAX;
    stats->max_ticks = 0U;
    stats->total_ticks = 0U;
}

void runtime_meter_stats_add(runtime_meter_stats_t *stats,
                             runtime_meter_tick_t ticks)
{
    if (stats == NULL || stats->count == UINT32_MAX
        || stats->total_ticks > UINT64_MAX - ticks) {
        return;
    }

    if (ticks < stats->min_ticks) {
        stats->min_ticks = ticks;
    }
    if (ticks > stats->max_ticks) {
        stats->max_ticks = ticks;
    }

    stats->total_ticks += ticks;
    ++stats->count;
}

runtime_meter_tick_t runtime_meter_stats_average_ticks(
    const runtime_meter_stats_t *stats)
{
    if (stats == NULL || stats->count == 0U) {
        return 0U;
    }

    return divide_u64_by_u32(stats->total_ticks, stats->count);
}
