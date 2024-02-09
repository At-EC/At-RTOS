# [At-RTOS](https://github.com/At-EC/At-RTOS) &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/At-EC/At-RTOS/blob/main/LICENSE) [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/At-EC/At-RTOS/tree/main/.github/Welcome_PRs.md) <a href="https://github.com/At-EC/At-RTOS/actions"><img alt="Build" src="https://github.com/At-EC/At-RTOS/workflows/Build/badge.svg"></a> <a href="https://github.com/At-EC/At-RTOS/stargazers"><img alt="GitHub stars" src="https://img.shields.io/github/stars/At-EC/At-RTOS?color=success"></a> <a href="https://github.com/At-EC/At-RTOS/issues"><img alt="GitHub issues" src="https://img.shields.io/github/issues/At-EC/At-RTOS"></a>

At-RTOS is a user-friendly embedded controller's real-time operating system designed specifically for the ARM Cortex M seiral architecture.

The goal of the project is to explore and try to provide a lot useful interfaces based on the RTOS to support and simplify your embedded firmware development.

If this project was useful to you, give it a ⭐️ and I'll keep improving it. moreover, you can share your ideas we can together improve it. Welcome PRs!!! 

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

## Kernal Structure

```shell
# At-RTOS source code tree
At-RTOS
├── arch
│   ├── arch32
│   │   └── arm
│   │       └── cmsis
│   │          └── include
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
│   │   ├── basic.h
│   │   ├── compiler.h
│   │   ├── configuration.h
│   │   ├── event.h
│   │   ├── idle.h
│   │   ├── kernal.h
│   │   ├── kernal_thread.h
│   │   ├── linker.h
│   │   ├── list.h
│   │   ├── member_struct.h
│   │   ├── mutex.h
│   │   ├── postcode.h
│   │   ├── queue.h
│   │   ├── semaphore.h
│   │   ├── thread.h
│   │   ├── timer.h
│   │   ├── trace.h
│   │   ├── type.h
│   │   └── unique.h
│   ├── basic.c
│   ├── event.c
│   ├── kernal.c
│   ├── kernal_thread.c
│   ├── linker.c
│   ├── list.c
│   ├── mutex.c
│   ├── queue.c
│   ├── semaphore.c
│   ├── thread.c
│   ├── timer.c
│   ├── trace.c
│   └── CMakeLists.txt
├── kernal_idle.c
└── CMakeLists.txt
```

## What's New

[v1.0.0] Welcome to At-RTOS. v1.0.0 was released now. A basic RTOS feature was implemented in the kernal system, Pls enjoy it (:

## Getting Started

The at_rtos.h is an interface of At-RTOS kernal. You can check the interface usage in this file to perform it in your embedded controller system.

## About Me
**Vision for the future:** The At-RTOS could be a popular community-based embedded controller's real-time operating system in the future and I'll continous to improve it.

### License
At-RTOS is [MIT licensed](./LICENSE).
