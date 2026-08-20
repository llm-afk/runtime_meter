# runtime_meter

`runtime_meter` 是面向 MCU 的小型程序段耗时测量组件。核心仅依赖 C99，
不使用动态内存，不依赖 RTOS、日志框架或具体芯片 SDK。

## 特点

- `start/stop` 两次调用即可测量，结果为硬件计数器 tick；
- 每次 `runtime_meter_init()` 都会重新校准测量开销，适应时钟和编译优化变化；
- 自动扣除校准开销，并正确处理 32 位计数器的一次自然回绕；
- 提供 tick 到 ns 的整数换算，以及可选的 `min/avg/max` 统计；
- 平台只需实现“初始化计数器”和“读取计数器”两个函数；
- 已在 N32H47x（Cortex-M4 DWT，240 MHz）实板验证。

## 使用

```c
#include "runtime_meter.h"

if (!runtime_meter_init()) {
    /* 硬件计数器不可用 */
}

runtime_meter_tick_t start = runtime_meter_start();
code_to_measure();
runtime_meter_tick_t ticks = runtime_meter_stop(start);
uint64_t ns = runtime_meter_ticks_to_ns(ticks);
```

核心 API 共 6 个：

```c
bool runtime_meter_init(void);
runtime_meter_tick_t runtime_meter_start(void);
runtime_meter_tick_t runtime_meter_stop(runtime_meter_tick_t start);
uint64_t runtime_meter_ticks_to_ns(runtime_meter_tick_t ticks);
uint32_t runtime_meter_frequency_hz(void);
runtime_meter_tick_t runtime_meter_overhead_ticks(void);
```

多次采样时可包含 `runtime_meter_stats.h`，使用调用者自己持有的统计对象；组件内部
不会分配内存。

## 移植

复制 `port/runtime_meter_port_template.c` 到平台层，只实现：

```c
bool runtime_meter_port_init(uint32_t *frequency_hz);
uint32_t runtime_meter_port_now(void);
```

仓库附带 `ports/n32h47x/runtime_meter_port_dwt.c` 作为 DWT 参考实现。其他 MCU、
通用定时器方案和验收要求见 [PORTING.md](PORTING.md)。

## CMake

```cmake
set(RUNTIME_METER_PORT_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/runtime_meter_port.c)
set(RUNTIME_METER_PORT_LIBRARIES platform_driver) # 可选
add_subdirectory(third_party/runtime_meter)
target_link_libraries(app PRIVATE runtime_meter::runtime_meter)
```

运行仓库自带的假计数器测试：

```sh
cmake -S . -B build -DRUNTIME_METER_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## 使用边界

- 单次测量必须短于计数器完整回绕周期；240 MHz 的 32 位计数器约为 17.9 秒。
- 修改系统时钟后再次调用 `runtime_meter_init()`。
- `init()` 不应与正在进行的测量并发；同一统计对象也不能被多个上下文无锁写入。
- 待测代码的结果必须可观察，避免被优化器删除；ns 换算应放在报告路径而非高速环路。
