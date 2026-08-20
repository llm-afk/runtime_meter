#ifndef RUNTIME_METER_PORT_H
#define RUNTIME_METER_PORT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enable and validate the selected free-running counter.
 *
 * On success, write the counter's actual ticks-per-second value to
 * frequency_hz and return true. The function must be safe to call repeatedly,
 * because runtime_meter_init() also uses it after clock changes.
 */
bool runtime_meter_port_init(uint32_t *frequency_hz);

/**
 * Read a monotonic, free-running 32-bit counter.
 *
 * Natural modulo-2^32 wrap is required. Keep this path as short as possible;
 * it must not reset, stop, or otherwise perturb the counter.
 */
uint32_t runtime_meter_port_now(void);

#ifdef __cplusplus
}
#endif

#endif
