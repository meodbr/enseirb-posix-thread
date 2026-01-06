# Custom POSIX Thread Library – ENSEIRB-MATMECA Student project

Despite the complexity, our library achieved **performance superior to pthread and other teams’ implementations**

## About this Project

This project is a **custom implementation of a POSIX-like thread library in C**, with full thread management, scheduling, and synchronization support. We implemented all the **basic functionalities** — thread creation, join, exit, yield, priorities, and mutexes — as well as a **complete scheduler** designed to balance CPU usage across threads efficiently.  

Key functionnalities I personally did:

- Implemented a **Black-Red Tree entirely in C macros** to schedule threads dynamically (I now know doing it in macro wasn't that usefull for speed efficency)
- Used **Time Stamp Counter (TSC) reading with inline assembly** for nanosecond-speed CPU time tracking, avoiding slower standard C functions (microsecond order) that rely on syscalls  
- Completed a **Highly-precise scheduler** that balances CPU time across threads accurately thanks to the black-red tree, preemption, and priority parts.

Despite the complexity, our library achieved **speed performance superior to POSIX standard lib** on most of the task given (thread creation, thread yielding, ect...). 
---

## Project Overview

The library provides a POSIX-like threading interface:

- `thread_create` – create a new thread  
- `thread_self` – get the current thread ID  
- `thread_yield` – voluntarily yield execution to another thread  
- `thread_join` – wait for a thread to finish and retrieve its return value  
- `thread_exit` – terminate the current thread  
- `thread_getpriority` / `thread_setpriority` – manage thread scheduling priorities  
- `thread_mutex_t` – basic mutex for synchronization  

Advanced scheduling features include:

- Priority-based scheduling with dynamic reordering  
- CPU-time tracking using TSC to balance compute across threads  
- Mutex queues to synchronize waiting threads efficiently  

---

## Installation

1. Clone the repository:

```bash
git clone <repo_url>
cd <repo_folder>
```

2. Compile the library and test programs:

```bash
make
```

- Use `make clean` to remove object files and binaries.

---

## Usage

```bash
./run_tests.sh
```

This script runs the tests given by our supervisors :

- Basic thread creation, yield, and join (`01-main.c`, `11-join.c`)  
- Thread scheduling and CPU time balance (`02-switch.c`, `03-equity.c`)  
- Creating multiple threads and recursive/thread-heavy scenarios (`21-create-many.c`, `22-create-many-recursive.c`)  
- Mutexes and synchronization (`61-mutex.c`, `62-mutex.c`, `63-mutex-equity.c`, `64-mutex-join.c`)  
- Preemption and priority handling (`71-preemption.c`, `91-priority.c`)  
- Deadlock detection (`81-deadlock.c`)  
- Special tests such as Fibonacci threads (`51-fibonacci.c`) and cascading joins (`33-switch-many-cascade.c`)  

This provides a **full demonstration of all implemented functionalities**, including the scheduler, mutexes, priorities, and preemption features.

For custom tests, include the library in your own C programs:

```c
#include "thread.h"
```

Then use the threading API:

```c
thread_t t;
thread_create(&t, my_function, NULL);
thread_join(t, NULL);
```

- Protect shared resources with `thread_mutex_t`.  
- Adjust thread priorities with `thread_setpriority` and `thread_getpriority`.  
- Use `thread_yield` for cooperative multitasking.  

## Notes

- This library is intended for **educational and performance exploration purposes**.  
- The scheduler prioritizes **balanced CPU time over raw speed**, using **nanosecond-precision TSC readings** for accurate measurements.  
- Optional features like preemption or more advanced scheduling can be implemented for further experimentation.

