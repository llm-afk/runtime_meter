#ifndef RUNTIME_METER_H
#define RUNTIME_METER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Override with a compiler definition when the default is not suitable. */
#ifndef RUNTIME_METER_CFG_CALIBRATION_SAMPLES
#define RUNTIME_METER_CFG_CALIBRATION_SAMPLES 64U
#endif

#if RUNTIME_METER_CFG_CALIBRATION_SAMPLES == 0U
#error "RUNTIME_METER_CFG_CALIBRATION_SAMPLES must be greater than zero"
#endif

typedef uint32_t runtime_meter_tick_t;

/**
 * Initialize the platform counter and calibrate measurement overhead.
 *
 * Every call performs a fresh calibration. Call once during boot, and call
 * again after changing the counter or system clock frequency.
 */
bool runtime_meter_init(void);

/** Capture the beginning of a measured interval. */
runtime_meter_tick_t runtime_meter_start(void);

/**
 * Finish an interval and return calibrated elapsed ticks.
 *
 * Handles one 32-bit counter wrap and never underflows while removing the
 * calibrated start/stop overhead.
 */
runtime_meter_tick_t runtime_meter_stop(runtime_meter_tick_t start);

/** Convert elapsed ticks to nanoseconds, rounded to nearest. */
uint64_t runtime_meter_ticks_to_ns(runtime_meter_tick_t ticks);

/** Return the active counter frequency, or zero before a successful init. */
uint32_t runtime_meter_frequency_hz(void);

/** Return the start/stop overhead measured by the latest successful init. */
runtime_meter_tick_t runtime_meter_overhead_ticks(void);

#ifdef __cplusplus
}
#endif

#endif
