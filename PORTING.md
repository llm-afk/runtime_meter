# runtime_meter 移植

移植时不要修改 `inc/` 和 `src/`。把
`port/runtime_meter_port_template.c` 复制到目标平台或 BSP 目录，例如：

```text
platform/stm32f10x/hal/runtime_meter_port_timer.c
platform/gd32c10x/hal/runtime_meter_port_dwt.c
```

然后只实现下面两个函数。

## 1. 初始化计数器

```c
bool runtime_meter_port_init(uint32_t *frequency_hz);
```

要求：

1. 检查 `frequency_hz != NULL`。
2. 启用一个单调递增、自由运行的 32 位硬件计数器。
3. 验证计数器确实会递增。
4. 返回该计数器真实的每秒 tick 数。
5. 支持重复调用，不能依赖只执行一次。

返回的必须是计数器频率，不一定是 CPU 主频。例如通用定时器经过分频后以 1 MHz
运行，就返回 `1000000U`。

## 2. 读取计数器

```c
uint32_t runtime_meter_port_now(void);
```

要求：

- 读取路径尽可能短；
- 允许 `uint32_t` 自然回绕；
- 不复位、不暂停计数器；
- 普通代码和 ISR 读取时都安全；
- 不在函数内部换算成 us/ns。

## Cortex-M DWT 方案

带 DWT `CYCCNT` 的 Cortex-M3/M4/M7 通常只需：

1. 打开 `CoreDebug->DEMCR.TRCENA`；
2. 检查芯片是否实现 `CYCCNT`；
3. 打开 `DWT->CTRL.CYCCNTENA`；
4. 验证 `DWT->CYCCNT` 会变化；
5. 返回 `SystemCoreClock`；
6. `runtime_meter_port_now()` 直接返回 `DWT->CYCCNT`。

仓库附带的 N32H47x 参考实现位于：

```text
ports/n32h47x/runtime_meter_port_dwt.c
```

不同 Cortex-M 芯片可能需要额外解锁 DWT，不能盲目复制寄存器序列，应根据对应
CMSIS 设备头和芯片手册确认。

## 通用定时器方案

没有 DWT 时，可配置一个 32 位定时器自由运行：

```c
bool runtime_meter_port_init(uint32_t *frequency_hz)
{
    timer_start_free_running();
    *frequency_hz = timer_input_hz / timer_prescaler;
    return timer_counter_is_running();
}

uint32_t runtime_meter_port_now(void)
{
    return TIMER_COUNTER_REGISTER;
}
```

只有 16 位定时器时，不建议直接移植；应先在 BSP 中可靠地扩展成 32 位单调计数器，
并处理溢出中断与并发读取。

## 接入构建系统

至少编译：

```text
runtime_meter/src/runtime_meter.c
platform/<target>/.../runtime_meter_port_xxx.c
```

并添加头文件目录：

```text
runtime_meter/inc
runtime_meter/port
```

CMake 工程可直接：

```cmake
set(RUNTIME_METER_PORT_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/<target>/runtime_meter_port_xxx.c)
set(RUNTIME_METER_PORT_LIBRARIES platform_driver)
add_subdirectory(middleware/runtime_meter)
target_link_libraries(app PRIVATE runtime_meter::runtime_meter)
```

`RUNTIME_METER_PORT_LIBRARIES` 是可选项，只在 port 依赖 CMSIS、芯片驱动库或其他
平台目标时设置。

## 移植验收

新平台至少验证：

- `runtime_meter_init()` 成功且频率正确；
- 连续两次读取产生非零校准开销；
- 已知延时的测量结果在允许误差内；
- 测量跨一次 `uint32_t` 回绕仍正确；
- 修改时钟后再次 `runtime_meter_init()` 能刷新频率和开销；
- Release 优化下结果没有被编译器删除。
