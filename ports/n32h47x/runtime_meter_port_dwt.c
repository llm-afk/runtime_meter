#include "runtime_meter_port.h"

#include <stddef.h>

#include "n32h47x_48x.h"
#include "system_n32h47x_48x.h"

bool runtime_meter_port_init(uint32_t *frequency_hz)
{
    uint32_t before;

    if (frequency_hz == NULL) {
        return false;
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    if ((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) != 0U) {
        return false;
    }

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    __DSB();
    __ISB();

    before = DWT->CYCCNT;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    if (DWT->CYCCNT == before) {
        return false;
    }

    SystemCoreClockUpdate();
    *frequency_hz = SystemCoreClock;
    return SystemCoreClock != 0U;
}

uint32_t runtime_meter_port_now(void)
{
    return DWT->CYCCNT;
}
