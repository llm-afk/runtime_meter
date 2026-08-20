#ifndef RUNTIME_METER_STATS_H
#define RUNTIME_METER_STATS_H

#include <stdint.h>

#include "runtime_meter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t count;
    runtime_meter_tick_t min_ticks;
    runtime_meter_tick_t max_ticks;
    uint64_t total_ticks;
} runtime_meter_stats_t;

/** Reset a caller-owned statistics accumulator. NULL is accepted. */
void runtime_meter_stats_reset(runtime_meter_stats_t *stats);

/** Add one calibrated duration. NULL is accepted. */
void runtime_meter_stats_add(runtime_meter_stats_t *stats,
                             runtime_meter_tick_t ticks);

/** Return the rounded-down average, or zero for NULL/an empty accumulator. */
runtime_meter_tick_t runtime_meter_stats_average_ticks(
    const runtime_meter_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif
