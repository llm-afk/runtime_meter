#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "runtime_meter.h"
#include "runtime_meter_port.h"
#include "runtime_meter_stats.h"

static bool s_port_available = true;
static uint32_t s_fake_counter;
static uint32_t s_fake_frequency_hz = 100000000U;
static uint32_t s_fake_read_cost = 7U;

bool runtime_meter_port_init(uint32_t *frequency_hz)
{
    if (!s_port_available || frequency_hz == NULL) {
        return false;
    }

    *frequency_hz = s_fake_frequency_hz;
    return true;
}

uint32_t runtime_meter_port_now(void)
{
    uint32_t value = s_fake_counter;
    s_fake_counter += s_fake_read_cost;
    return value;
}

static void fake_advance(uint32_t ticks)
{
    s_fake_counter += ticks;
}

static void test_init_and_measurement(void)
{
    runtime_meter_tick_t start;

    s_fake_counter = 0U;
    s_fake_frequency_hz = 100000000U;
    s_fake_read_cost = 7U;
    s_port_available = true;

    assert(runtime_meter_init());
    assert(runtime_meter_frequency_hz() == 100000000U);
    assert(runtime_meter_overhead_ticks() == 7U);

    start = runtime_meter_start();
    assert(runtime_meter_stop(start) == 0U);

    start = runtime_meter_start();
    fake_advance(123U);
    assert(runtime_meter_stop(start) == 123U);
    assert(runtime_meter_ticks_to_ns(123U) == 1230U);

    s_fake_read_cost = 1U;
    start = runtime_meter_start();
    assert(runtime_meter_stop(start) == 0U);
    s_fake_read_cost = 7U;
}

static void test_wraparound(void)
{
    runtime_meter_tick_t start;

    s_fake_counter = UINT32_MAX - 3U;
    start = runtime_meter_start();
    fake_advance(10U);
    assert(runtime_meter_stop(start) == 10U);
}

static void test_reinit_and_failure(void)
{
    s_fake_frequency_hz = 240000000U;
    s_fake_read_cost = 11U;
    assert(runtime_meter_init());
    assert(runtime_meter_frequency_hz() == 240000000U);
    assert(runtime_meter_overhead_ticks() == 11U);
    assert(runtime_meter_ticks_to_ns(1U) == 4U);

    s_port_available = false;
    assert(!runtime_meter_init());
    assert(runtime_meter_frequency_hz() == 0U);
    assert(runtime_meter_overhead_ticks() == 0U);
    assert(runtime_meter_ticks_to_ns(100U) == 0U);

    s_port_available = true;
    s_fake_frequency_hz = 0U;
    assert(!runtime_meter_init());
    assert(runtime_meter_frequency_hz() == 0U);
}

static void test_time_conversion(void)
{
    s_fake_frequency_hz = 3U;
    s_fake_read_cost = 1U;
    assert(runtime_meter_init());

    assert(runtime_meter_ticks_to_ns(0U) == 0U);
    assert(runtime_meter_ticks_to_ns(1U) == 333333333ULL);
    assert(runtime_meter_ticks_to_ns(2U) == 666666667ULL);
    assert(runtime_meter_ticks_to_ns(3U) == 1000000000ULL);
    assert(runtime_meter_ticks_to_ns(UINT32_MAX) == 1431655765000000000ULL);
}

static void test_statistics(void)
{
    runtime_meter_stats_t stats;

    runtime_meter_stats_reset(&stats);
    assert(stats.count == 0U);
    assert(runtime_meter_stats_average_ticks(&stats) == 0U);

    runtime_meter_stats_add(&stats, 30U);
    runtime_meter_stats_add(&stats, 10U);
    runtime_meter_stats_add(&stats, 20U);
    assert(stats.count == 3U);
    assert(stats.min_ticks == 10U);
    assert(stats.max_ticks == 30U);
    assert(runtime_meter_stats_average_ticks(&stats) == 20U);

    runtime_meter_stats_reset(NULL);
    runtime_meter_stats_add(NULL, 1U);
    assert(runtime_meter_stats_average_ticks(NULL) == 0U);

    stats.count = UINT32_MAX;
    stats.total_ticks = 123U;
    runtime_meter_stats_add(&stats, 1U);
    assert(stats.count == UINT32_MAX);
    assert(stats.total_ticks == 123U);

    stats.count = 1U;
    stats.min_ticks = 5U;
    stats.max_ticks = 5U;
    stats.total_ticks = UINT64_MAX - 4U;
    runtime_meter_stats_add(&stats, 5U);
    assert(stats.count == 1U);
    assert(stats.min_ticks == 5U);
    assert(stats.max_ticks == 5U);
    assert(stats.total_ticks == UINT64_MAX - 4U);
}

int main(void)
{
    test_init_and_measurement();
    test_wraparound();
    test_reinit_and_failure();
    test_time_conversion();
    test_statistics();
    puts("runtime_meter tests: PASS");
    return 0;
}
