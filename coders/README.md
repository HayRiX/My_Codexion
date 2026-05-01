*This project has been created as part of the 42 curriculum by [aryahi]*

# Codexion: High-Performance Resource Orchestration 

## Description
**Codexion** is an advanced concurrency challenge inspired by the classic Dining Philosophers problem, tailored for the 42 Network environment. The goal of this project is to simulate a high-pressure coding hub where multiple **Coders** (threads) compete for a limited set of **USB Dongles** (shared resources) to compile their code. The project requires strict adherence to deterministic scheduling algorithms (FIFO and EDF) using POSIX threads, while mathematically preventing deadlocks, starvation, and data races. 

This specific implementation goes beyond basic requirements by utilizing a **Min-Heap (Priority Queue)** for $O(\log N)$ scheduling efficiency, ensuring robust performance even under extreme stress tests (e.g., 200+ concurrent threads).

## Instructions
### Compilation
To compile the project and generate the executable, run the following command at the root of the repository:
```bash
make
```
*(You can also use `make clean`, `make fclean`, and `make re` for standard cleanup and recompilation).*

### Execution
The program requires 8 specific arguments to run the simulation:
```bash
./codexion [number_of_coders] [time_to_burnout] [time_to_compile] [time_to_debug] [time_to_refactor] [number_of_compiles_required] [dongle_cooldown] [scheduler_type]
```

**Example Usage:**
```bash
./codexion 5 800 200 200 200 7 10 edf
```
*Note: The `scheduler_type` must be either `fifo` or `edf`.*

## Blocking cases handled
Our solution rigorously addresses all classic concurrency pitfalls:
* **Deadlock Prevention & Coffman's Conditions:** We break the "Circular Wait" condition by implementing a strict resource hierarchy. Coders must always acquire their lowest-indexed dongle (`min_dongle`) before attempting to acquire the highest-indexed one (`max_dongle`).
* **Starvation Prevention:** We implemented an **Earliest Deadline First (EDF)** scheduler backed by a Min-Heap priority queue. To handle edge cases where deadlines are identical, a custom tie-breaker guarantees priority to the coder with the higher ID, completely eliminating starvation.
* **Cooldown Handling:** Dongle availability isn't just binary. We track cooldown timestamps in a dedicated array. A coder cannot acquire a dongle if the current time is less than the dongle's calculated cooldown threshold.
* **Precise Burnout Detection:** A separate, dedicated Monitor thread continuously polls the `last_compile_start` timestamp of each coder using a granular `coder_mutex` to ensure no coder burns out unnoticed.
* **Log Serialization:** All terminal outputs are funneled through a global `print_mutex` to ensure that timestamps remain chronological and messages are never garbled or interleaved.
* **Thundering Herd & Lock Contention Mitigation:** Prevented CPU spikes and excessive lock contention by optimizing the Monitor thread. Instead of spamming `pthread_cond_broadcast` every millisecond, the monitor only signals on actual state changes. Additionally, waiting threads employ intelligent microsleeps (`usleep`) when waiting strictly for cooldowns, ensuring 0% CPU waste during idle blocking.
* **Starvation Cascade Prevention:** Modified the `has_conflict` logic to evaluate the actual lock state (`dongle_states`) of higher-priority threads. A coder will only yield priority if the higher-priority coder is genuinely capable of acquiring resources, completely breaking the "chain of waiting" and allowing maximum parallel throughput.

## Thread Synchronization Mechanisms
To coordinate access to shared resources and ensure thread-safe communication, we utilized specific POSIX threading primitives:
* **`pthread_mutex_t`:** * `dongles[i]`: Represents the physical USB dongles. Locked when a coder acquires the resource.
  * `queue_mutex`: Protects the Min-Heap priority queue, the simulation state, and the dongle states. This ensures that scheduling operations and state reads/writes (via Getters/Setters) never result in Data Races.
  * `coder_mutex`: A per-coder mutex that strictly protects the `last_compile_start` and `compile_count` variables from being read by the Monitor while being updated by the Coder.
* **`pthread_cond_t`:** * `queue_cond`: Instead of inefficient "busy-waiting" (spinlocks), coders use `pthread_cond_wait` to sleep peacefully. When a coder drops their dongles, or when the Monitor detects a burnout/completion, a `pthread_cond_broadcast` is fired to awaken sleeping threads immediately, allowing for rapid state evaluation and clean exits.

## Profiling & Memory Safety
The architecture has been rigorously tested to handle extreme concurrency loads without memory leaks or race conditions.
* **Valgrind Mass-Thread Testing:** Successfully verified with `--max-threads=1500` running 1000 concurrent coders, returning strictly `0 errors` and `0 leaks` (max coders for safety testing is 57-56 coders max).
* **ThreadSanitizer & Helgrind:** Utilized to ensure absolute thread safety, verifying the correct placement of mutexes around all shared state mutations and reads.

## Resources
**Classic References:**
* *POSIX Threads Programming* - Lawrence Livermore National Laboratory (LLNL) documentation.
* *Advanced Programming in the UNIX Environment (APUE)* by W. Richard Stevens.
* Operating Systems Concepts (Silberschatz) - specifically chapters on Deadlocks and Process Synchronization.

**AI Usage:**
Artificial Intelligence (Claude 3.5 Sonnet / Gemini Advanced) was utilized during the development of this project for the following specific tasks:
1. **Refactoring & Optimization:** Transforming the initial $O(N^2)$ Bubble Sort scheduling logic into a high-performance $O(\log N)$ Binary Min-Heap (`heapify_up` and `heapify_down`).
2. **Data Race Debugging:** Analyzing `ThreadSanitizer` reports to identify unprotected reads of the `simulation_state` boolean, leading to the implementation of thread-safe Getter/Setter wrappers.
3. **Norminette Compliance:** Assisting in splitting large functions (like `can_take_dongles`) into smaller static helper functions to strictly adhere to the 42 Network's 25-line limit without breaking the logic.