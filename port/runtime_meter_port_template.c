/* Copy this file into your platform/BSP directory; do not edit the component. */

#include "runtime_meter_port.h"

#include <stddef.h>

bool runtime_meter_port_init(uint32_t *frequency_hz)
{
    if (frequency_hz == NULL) {
        return false;
    }

    /*
     * 1. Enable a monotonic free-running 32-bit hardware counter.
     * 2. Verify that it advances.
     * 3. Return its actual ticks-per-second frequency.
     */
    *frequency_hz = 0U;
    return false;
}

uint32_t runtime_meter_port_now(void)
{
    /* Return the current counter value. */
    return 0U;
}
