# [At-RTOS](https://github.com/At-EC/At-RTOS) &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/At-EC/At-RTOS/blob/main/LICENSE) [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/At-EC/At-RTOS/tree/main/.github/Welcome_PRs.md) <a href="https://github.com/At-EC/At-RTOS/actions"><img alt="Build" src="https://github.com/At-EC/At-RTOS/workflows/Build/badge.svg"></a> <a href="https://github.com/At-EC/At-RTOS/stargazers"><img alt="GitHub stars" src="https://img.shields.io/github/stars/At-EC/At-RTOS?color=success"></a> <a href="https://github.com/At-EC/At-RTOS/issues"><img alt="GitHub issues" src="https://img.shields.io/github/issues/At-EC/At-RTOS"></a>

At-RTOS is a user-friendly embedded controller's real-time operating system designed specifically for the ARM Cortex M seiral architecture.

The goal of the project is to explore and try to provide a lot useful interfaces based on the RTOS to support and simplify your embedded firmware development.

I hope the At-RTOS could be a popular community-based embedded controller's real-time operating system in the future. If this project was useful to you, give it a ⭐️ and I'll keep improving it. moreover, you can share your ideas we can together improve it. Welcome PRs!!! 

<p align="center">
<img src="https://socialify.git.ci/At-EC/At-RTOS/image?description=1&descriptionEditable=At-RTOS%20is%20an%20open%20user-friendly%20real-time%20operating%20system.&font=KoHo&forks=1&issues=1&name=1&owner=1&pattern=Circuit%20Board&stargazers=1&theme=Dark" alt="At-RTOS" width="640" height="320" />
</p>

## Introduction

* **Open source:** Royalty free.
* **Tickless:** At-RTOS makes it painless to create battery-powered application. 
* **Preemptive and Cooperative Scheduling:** You can easily config your task to pass preemptive and cooperative scheduling through your thread's priority setting.
* **Resource Mutexes:** It helps to protect your globally sensitive data from tampering by other threads.
* **Binary and Counting Semaphores:** It provides selectable counting and binary semaphore for thread communication in the system.
* **Queue Messages:** It's for thread-safe communication.
* **Multiple Events:** It's for thread-safe communication.
* **Software Timers with callback:** It supports your varity time requirements application.
* **Fully configurable (ROM and RAM):** No limits on number of At-RTOS objects, except your devices' available memory.
* **Tiny footprint:** It's as low as 1KB ROM/few bytes of RAM.
* **Learn Once, Write Anywhere:** We don't make assumptions about the rest of your technology stack, so you can develop new features in At-RTOS without rewriting existing code.

# Resources

## Supported Architectures

At-RTOS supports many architectures, and has covered the major architectures in current kernal system. It supports the following architectures, and it lists the chip that was verified.

- **ARM Cortex-M3 (armv7m)**: `TODO`
- **ARM Cortex-M4 (armv7em)**: `GD32F307VET6`
- **ARM Cortex-M23 (armv7em_f4sh)**: `TODO`
- **ARM Cortex-M33 (armv7em_f5sh and armv7em_f5dh)**: `TODO`

There is planned support for the ARM Cortex M3 and M23 architectures though no chips are currently supported in my hand, If you perform it in your system, I'll thanks for your PRs to update the chip that verified into lists.

## Supported IDE and Compiler

The main IDE/compilers supported by At-RTOS are:

- MDK KEIL: `PASS`
- IAR: `TODO`
- Native GCC: `PASS`
- ARM GCC: `TODO`

## Code Structure

```shell
# At-RTOS important source code tree is shown as follow
At-RTOS
├── arch
│   ├── arch32
│   │   └── arm
│   │       └── cmsis
│   │          └── include
│   │              └── *.h
│   ├── arch.h
│   └── CMakeLists.txt
├── clock
│   ├── include
│   │   └── clock_tick.h
│   ├── clock_systick.c
│   └── CMakeLists.txt
├── port
│   ├── include
│   │   └── port.h
│   ├── port_keil.c
│   └── CMakeLists.txt
├── include
│   ├── atos_configuration.h
│   └── CMakeLists.txt
├── kernal
│   ├── include
│   │   ├── at_rtos.h
│   │   ├── atos_version.h
│   │   └── *.h
│   ├── kernal.c
│   ├── *.c
│   └── CMakeLists.txt
├── kernal_idle.c
└── CMakeLists.txt
```

- **arch :** This folder was used to store all the chip architectures supported by the current At-RTOS.
- **clock :** It was implemented for At-RTOS kernal system tick to support system timers.
- **port :** It's used to support different compiler such as KEIL, IAR and GCC.
- **include :** It used to contain the At-RTOS kernal header file such as the At-RTOS user configurations.
- **kernal :** This folder was implemented for the At-RTOS kernal files.

## ARM Cortex M architecture

At-RTOS was designed specifically to take advantage of the powerful hardware features introduced with the ARM Cortex M architecture. The following HW resources were used in the At-RTOS kernal.

- Nested Vectored Interrupt Controller (NVIC)
- Service Call (SVC instruction)
- SysTick
- PendSV
- Floating Point Unit (FPU)
- Memory Protection Unit (MPU)
- Thread Mode/Handler Mode

# Getting Started

The at_rtos.h is an interface of At-RTOS kernal. You can check the interface usage in this file to perform it in your embedded controller system.

## Configuration

At-RTOS ported a template At-RTOS configuration header file `<root_path>/include/atos_configuration.h`. Your board support package must provide the following variable symbols to instead of this one.
```c
/**
 * If you are use ARM Cortex M seiral architecture and use the system tick as the kernal timer.
 * In most cases, PORTAL_SYSTEM_CORE_CLOCK_MHZ must be set to the frequency of the clock
 * that drives the peripheral used to generate the kernels periodic tick interrupt.
 * The default value is set to 120mhz.
 */
#define PORTAL_SYSTEM_CORE_CLOCK_MHZ              (120u)
```
Your application will certainly need a different value so set the kernal component instance number correctly. This is very often, but not always. It's according to your system design.
The symbols in the configuration header file look like this `<kernal component>_INSTANCE_SUPPORTED_NUMBER`, and the kernal component is shown as following table:
- Thread
- SEMAPHORE
- EVENT
- MUTEX
- QUEUE
- TIMER
- ...

The more details you can see the descriptions in the file `<root_path>/include/atos_configuration.h`.

## Interface Usage

Create Your First Thread is shown as follow:
```c
/* Include the At-RTOS interface's header file. */
#include "at_rtos.h"

/* Define a thread hook to specific the stack size and prioriy of the thread */
ATOS_THREAD_DEFINE(first_thread, 1024, 7); // Set the thread stack size to 1024 bytes and the schedule prioriy level to 7.

/* User thread's entry function. */
static void first_thread_entry(void)
{
    while (1)
    {
        AtOS.thread_sleep(1000u);
    }
}

/* The main routine */
int main(void)
{
    /* Initialize the your first thread. */
    os_thread_id_t first_id = AtOS.thread_init(first_thread, first_thread_entry);
    if (AtOS.id_isInvalid(first_id))
    {
        printf("Thread %s init failed\n", first_id.pName);
    }
	
    /* The At-RTOS kernal schedule starts to run. */
    AtOS.at_rtos_run();
}
```

## What's New

[v1.0.0] Welcome to At-RTOS. v1.0.0 was released now. A basic RTOS feature was implemented in the kernal system, Pls enjoy it (:

## License
At-RTOS is [MIT licensed](./LICENSE).

The At-RTOS is completely open-source, can be used in commercial applications for free, does not require the disclosure of code, and has no potential commercial risk. License information and copyright information can generally be seen at the beginning of the code:

```c
/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
