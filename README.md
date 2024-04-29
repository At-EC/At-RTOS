# [At-RTOS](https://github.com/At-EC/At-RTOS) &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/At-EC/At-RTOS/blob/main/LICENSE) [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/At-EC/At-RTOS/tree/main/.github/Welcome_PRs.md) <a href="https://github.com/At-EC/At-RTOS/stargazers"><img alt="GitHub stars" src="https://img.shields.io/github/stars/At-EC/At-RTOS?color=success"></a> [![Discord](https://img.shields.io/discord/1208405601994932274?logo=discord)](https://discord.gg/AxfF9aH58G) <a href="https://github.com/At-EC/At-RTOS/actions"><img alt="Build" src="https://github.com/At-EC/At-RTOS/workflows/Build/badge.svg"></a> [![GitHub Release](https://img.shields.io/github/v/release/At-EC/At-RTOS)](./release-note.md)

At-RTOS([Advantage Real-Time Operating System](https://github.com/At-EC/At-RTOS)) is an open and user-friendly Real-Time Operating System(RTOS) designed for various embedded controllers' supporting, as well it's beginning that will be an important member of the At-EC family.

The goal of the system is to explore and try to provide a range of features and capabilities based on the RTOS to support and simplify your embedded firmware development.

We hope the At-RTOS could be a popular community-based embedded controller's real-time operating system in the future. If this project was useful to you, give it a ⭐. It is very important for us to know it is useful to you and your attention will encourage us. 

Welcome PRs! If you are interested in contributing, Pls join us at [Discord](https://discord.gg/AxfF9aH58G). 

<p align="center">

<img src="https://socialify.git.ci/At-EC/At-RTOS/image?description=1&descriptionEditable=Advantage%20Real-Time%20Operating%20System&font=KoHo&issues=1&name=1&owner=1&pattern=Circuit%20Board&pulls=1&stargazers=1&theme=Auto" alt="At-RTOS" width="640" height="320" />

</p>

## What's news

   At-RTOS branch and release news as shown as the following table:

| Version Name          | Description                                                                  |
| ------------- | ------------------------------------------------------------------------------------ |
| [![GitHub Release](https://img.shields.io/github/v/release/At-EC/At-RTOS)](./release-note.md) | Welcome to At-RTOS. This production version was released which supports a stable RTOS feature was implemented in the kernel system, Pls enjoy it (: |
| ![GitHub package.json version (branch)](https://img.shields.io/github/package-json/v/At-EC/At-RTOS/main) | The development version in the main branch was implemented which will support new features, but it probably has unknown issues. |

The release note is [here](https://github.com/At-EC/At-RTOS/releases).

## Introduction

At-RTOS is a type of Real-Time Operating System that focuses on adaptability and flexibility to cater to the needs of various embedded controllers. It's used to manage and control the hardware and firmware resources of embedded systems, ensuring real-time thread execution and system stability.

It provides a range of features and capabilities such as thread scheduling, messages routing and interrupt handling, and more, to meet the requirements of embedded systems for real-time performance and reliability. It supports concurrent execution of multiple threads, enabling the management of multiple threads while ensuring synchronization and communication between them. 

It's also offers extensive APIs(Application Programming Interfaces) that allow developers to configure the system, create threads, communicate betwwen threads, customize kernel clock, and more, with ease. Additionally At-RTOS supports various hardware platforms and compiilers specifically for the ARM Cortex M seiral architecture, offering good portability and scalability.

Overall, At-RTOS is powerful, flexible, and scalable real-time operating system suitable for the development and deployment of various embbedded systems. For more information on At-RTOS, it is recommended to refer to relevant technical documentation and sample codes.

And the keywords of the At-RTOS is shown as following lists.

* **Open source:** Royalty free.
* **Tickless:** At-RTOS makes it painless to create battery-powered application. 
* **Preemptive and Cooperative Scheduling:** You can easily config your thread to pass preemptive and cooperative scheduling through your thread's priority setting.
* **Resource Mutexes:** It helps to protect your globally sensitive data from tampering by other threads.
* **Binary and Counting Semaphores:** It provides selectable counting and binary semaphore for thread communication in the system.
* **Queue Messages:** It's for thread-safe communication.
* **Multiple Events:** It's for thread-safe communication.
* **Memory Pools:** It's for thread memory pool resource management.
* **Software Timers with callback:** It supports your varity time requirements application.
* **Fully configurable (ROM and RAM):** No limits on number of At-RTOS objects, except your devices' available memory.
* **Tiny footprint:** It has low ROM/RAM consumption.
* **Learn Once, Write Anywhere:** We don't make assumptions about the rest of your technology stack, so you can develop new features in At-RTOS without rewriting existing code.

## Cores

At-RTOS supports many architectures, and has covered the major architectures in current kernel system. It supports the following architectures, and it lists the chip that was verified.

- [ ] `ARM Cortex-M` lists:
    - [x] Cortex-M3: [GD32103C-START](https://www.gigadevice.com/product/mcu/arm-cortex-m3/gd32f103vct6)
    - [x] Cortex-M4: [GD32F307E-START](https://www.gigadevice.com/product/mcu/arm-cortex-m4/gd32f307vet6)
    - [ ] Cortex-M23:
    - [x] Cortex-M33: [GD32W515P-EVAL](https://www.gigadevice.com/product/mcu/arm-cortex-m33/gd32w515piq6)
- [ ] `RISC-V` lists:
    - [ ] Reserved:

There is planned support for the ARM Cortex remaining architectures though no chips are currently supported in my hand, If you perform it in your system, I'll thanks for your PRs to update the chip that verified into lists.

## Compilers

The main compilers currently is supported by At-RTOS are:

- [ ] `Compiler` lists:
    - [x] MDK KEIL (AC5).
    - [x] MDK KEIL (AC6).
    - [x] IAR IDE.
    - [x] ARMCLANG.
    - [ ] CMAKE ARM GCC.
    - [x] CMAKE Native GCC.

## Source tree

```shell
# At-RTOS important source code tree is shown as follow
At-RTOS
├── arch
│   └── arch32
├── clock
│   ├── clock_systick.c
│   └── CMakeLists.txt
├── port
│   ├── port.c
│   ├── port_keil_ac5.c
│   ├── port_keil_ac6.c
│   ├── port_iar.s
│   └── CMakeLists.txt
├── include
│   ├── kernel
│   │   ├── at_rtos.h
│   │   └── *.h
│   ├── arch.h
│   ├── port.h
│   ├── clock_tick.h
│   └── CMakeLists.txt
├── kernel
│   ├── kernel.c
│   ├── *.c
│   └── CMakeLists.txt
├── build_version.h
└── CMakeLists.txt
```

- **arch :** This folder provided the chip core architectures resource to support At-RTOS kernel feature.
- **clock :** It was implemented for At-RTOS kernel system tick to support system timers.
- **port :** It's used to support different compiler such as KEIL, IAR and GCC.
- **include :** It used to contain the At-RTOS kernel header files, Moreover it contained the portable arch, clock and port header files.
- **kernel :** This folder was implemented for the At-RTOS kernel files.

## Invoked resources

At-RTOS was designed specifically to take advantage of the powerful hardware features introduced with the ARM Cortex M architecture. The following HW resources were used in the At-RTOS kernel.

- Nested Vectored Interrupt Controller (NVIC)
- Service Call (SVC instruction)
- SysTick
- PendSV
- Floating Point Unit (FPU)
- Memory Protection Unit (MPU)
- Thread Mode/Handler Mode

# Getting Started

## User configuration

At-RTOS ported a template At-RTOS configuration header file [atos_configuration.h](./include/template/atos_configuration.h). Your board support package must provide the following variable symbols to instead of this one.

```c
/**
 * If you are use ARM Cortex M seiral architecture, the Cortex-M Core architecture must be declared as the following list.
 * 
 * ARCH_ARM_CORTEX_CM3
 * ARCH_ARM_CORTEX_CM4
 * ARCH_ARM_CORTEX_CM33
 **/
#define ARCH_ARM_CORTEX_CM33

/**
 * If you are use ARM Cortex M seiral architecture and use the system tick as the kernel timer.
 * In most cases, PORTAL_SYSTEM_CORE_CLOCK_MHZ must be set to the frequency of the clock
 * that drives the peripheral used to generate the kernels periodic tick interrupt.
 * The default value is set to 120mhz.
 */
#define PORTAL_SYSTEM_CORE_CLOCK_MHZ (120u)
```

Your application will certainly need a different value so set the kernel component instance number correctly. This is very often, but not always. It's according to your system design.
The symbols in the configuration header file look like this `<kernel component>_INSTANCE_SUPPORTED_NUMBER`, and the kernel component is shown as following table:
- THREAD
- SEMAPHORE
- EVENT
- MUTEX
- QUEUE
- TIMER
- POOL
- ...

The more details you can see the descriptions in the template file [atos_configuration.h](./include/template/atos_configuration.h).

## User interface

The [at_rtos.h](./include/kernel/at_rtos.h) is an interface of At-RTOS kernel. You can check the interface usage in this file to perform it in your embedded controller system.

The following sample codes illustrates how to create your first thread:
```c
/* Include the At-RTOS interface's header file. */
#include "at_rtos.h"

/* Define a thread hook to specific the stack size and prioriy of the thread */
OS_THREAD_DEFINE(your_thread_define, 1024, 7); // Set the thread stack size to 1024 bytes and the schedule prioriy level to 7.

/* User thread's entry function. */
static void your_thread_entry_function(void)
{
    while(1) {
        os.thread_sleep(1000u);
    }
}

/* The main routine */
int main(void)
{
    /* Initialize the your your thread. */
    os_thread_id_t your_thread_id = os.thread_init(your_thread_define, your_thread_entry_function);
    if (os.id_isInvalid(your_thread_id)) {
        printf("Thread %s init failed\n", your_thread_id.pName);
    }
	
    /* The At-RTOS kernel schedule starts to run. */
    os.schedule_run();
}
```

The following kernel H file path must be included in your project workshop.

```shell
<root path>\
<root path>\include\
<root path>\include\kernel\
```
The following kernel C file should be placed in your project workshop based on your chip feature and compiler.

```shell
<root path>\kernel\<all of them>.c
<root path>\clock\clock_systick.c
<root path>\port\port_common.c
<root path>\port\<your compiler>.c
```

## Roadmap

The At-EC roadmap documentation is not ready. The At-RTOS is a beginning and basic component and will be an important member of the At-EC family.

## Contribution

The contributing documentation is not ready, You can check the open issue right now that we're developing.

![Alt](https://repobeats.axiom.co/api/embed/7437f309b3c464047a0adedf4e5a2c916df10cdc.svg "Repobeats analytics image")

## License

The At-RTOS is completely open-source, can be used in commercial applications for free, does not require the disclosure of code, and has no potential commercial risk. License information and copyright information can generally be seen at the beginning of the code:

```c
/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/
```

The MIT License is [here](./LICENSE).
