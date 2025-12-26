# Architecture Diagrams

## Thread Interaction Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                         MAIN THREAD                              │
│                                                                  │
│  Responsibilities:                                               │
│  • Initialize SharedBuffer                                       │
│  • Create all thread objects                                     │
│  • Start producer and consumer threads                           │
│  • Monitor for shutdown signal (Ctrl+C or timeout)               │
│  • Coordinate graceful shutdown                                  │
│  • Display final performance report                              │
└────────────┬─────────────────────────────────────────────────────┘
             │
             │ Creates & Manages
             │
    ┌────────┴────────┬─────────────┬──────────────┐
    │                 │             │              │
    ▼                 ▼             ▼              ▼
┌─────────┐     ┌──────────┐  ┌──────────┐  ┌──────────────┐
│Producer │     │Consumer 1│  │Consumer 2│  │  Consumer 3  │
│ Thread  │     │ (Display)│  │  (SMA)   │  │(Volatility)  │
│         │     │          │  │          │  │              │
│ Updates │     │ Refresh  │  │ Calculates│ │  Calculates  │
│ every   │     │ every    │  │ every    │  │  every       │
│ 100ms   │     │ 500ms    │  │ 1000ms   │  │  1500ms      │
└────┬────┘     └────▲─────┘  └────▲─────┘  └──────▲───────┘
     │               │             │                │
     │ push()        │ getLatest() │ getHistory()   │ getHistory()
     │               │             │                │
     └───────────────┴─────────────┴────────────────┘
                            │
                            ▼
              ┌───────────────────────────┐
              │     SHARED BUFFER         │
              │   (Thread-Safe)           │
              │                           │
              │  ┌─────────────────────┐  │
              │  │  std::mutex         │  │ ← Protects all access
              │  ├─────────────────────┤  │
              │  │  std::condition_    │  │ ← Signals new data
              │  │  variable           │  │
              │  ├─────────────────────┤  │
              │  │  Circular Buffer    │  │ ← Stores price history
              │  │  (per symbol)       │  │
              │  │  Max: 100 ticks     │  │
              │  └─────────────────────┘  │
              └───────────────────────────┘
```

## Synchronization Flow

### Producer (Price Generator) Flow

```
Producer Thread:
┌────────────────┐
│ Generate Price │
└───────┬────────┘
        │
        ▼
┌─────────────────────┐
│ Create PriceData    │
│ with timestamp      │
└───────┬─────────────┘
        │
        ▼
┌─────────────────────────────────┐
│ Call SharedBuffer::push()       │
│                                 │
│  1. Acquire mutex (lock)        │  ← CRITICAL SECTION START
│  2. Add data to circular buffer │
│  3. Update statistics           │
│  4. Release mutex (unlock)      │  ← CRITICAL SECTION END
│  5. Notify condition variable   │  ← Wake waiting consumers
└───────┬─────────────────────────┘
        │
        ▼
┌────────────────┐
│ Sleep 100ms    │
└───────┬────────┘
        │
        ▼
┌────────────────┐
│ Repeat         │
└────────────────┘
```

### Consumer (Calculator) Flow

```
Consumer Thread (SMA/Volatility):
┌────────────────────────────────┐
│ Call SharedBuffer::waitForData()│
│                                │
│  1. Acquire mutex (lock)       │
│  2. Wait on condition variable │  ← Sleep until signaled
│     (releases lock while wait) │
│  3. Woken by producer signal   │
│  4. Re-acquire mutex           │
│  5. Check data availability    │
│  6. Release mutex (unlock)     │
└───────┬────────────────────────┘
        │
        ▼
┌─────────────────────────────────┐
│ Call SharedBuffer::getHistory() │
│                                 │
│  1. Acquire mutex (lock)        │  ← CRITICAL SECTION START
│  2. Read price history          │
│  3. Release mutex (unlock)      │  ← CRITICAL SECTION END
└───────┬─────────────────────────┘
        │
        ▼
┌─────────────────────┐
│ Calculate Indicator │  ← No locks needed (local computation)
│ (SMA or Volatility) │
└───────┬─────────────┘
        │
        ▼
┌─────────────────────┐
│ Record Performance  │
│ Metrics             │
└───────┬─────────────┘
        │
        ▼
┌─────────────────────┐
│ Sleep interval      │
└───────┬─────────────┘
        │
        ▼
┌─────────────────────┐
│ Repeat              │
└─────────────────────┘
```

## Data Flow Diagram

```
Stock Symbols: [AAPL, GOOGL, MSFT, AMZN, BTC]
                        ↓
    ┌───────────────────────────────────────┐
    │     Price Generator (Producer)        │
    │  • Random price changes               │
    │  • Normal distribution (μ=0, σ=0.5)   │
    │  • High-resolution timestamp          │
    └─────────────────┬─────────────────────┘
                      │
                      │ PriceData{symbol, price, change, timestamp}
                      ↓
    ┌───────────────────────────────────────┐
    │        Shared Circular Buffer         │
    │  • Thread-safe (mutex protected)      │
    │  • Per-symbol history (max 100)       │
    │  • Condition variable signaling       │
    └────┬──────────────┬──────────────┬────┘
         │              │              │
         ↓              ↓              ↓
    ┌────────┐   ┌──────────┐   ┌──────────┐
    │Display │   │   SMA    │   │Volatility│
    │Consumer│   │Calculator│   │Calculator│
    └────┬───┘   └─────┬────┘   └─────┬────┘
         │             │              │
         ↓             ↓              ↓
    ┌────────┐   ┌──────────┐   ┌──────────┐
    │Console │   │   MA     │   │   σ      │
    │Output  │   │ Indicator│   │Indicator │
    └────────┘   └──────────┘   └──────────┘
                       │              │
                       └──────┬───────┘
                              ↓
                   ┌──────────────────────┐
                   │ Performance Monitor  │
                   │ • Latency tracking   │
                   │ • Statistics         │
                   └──────────────────────┘
```

## Mutex Lock Ordering (Deadlock Prevention)

```
RULE: Only ONE mutex exists in the system → No deadlock possible

SharedBuffer has single mutex:
┌─────────────────────────────────────────┐
│         SharedBuffer::mutex_            │
│                                         │
│  Protected Resources:                   │
│  • price_history_ (all price data)     │
│  • total_writes_ (counter)              │
│  • total_reads_ (counter)               │
│  • shutdown_ (flag)                     │
└─────────────────────────────────────────┘
         ▲                    ▲
         │                    │
    All threads          No circular wait
    use same mutex       = No deadlock!
```

## Memory Layout

```
Per-Symbol Circular Buffer (example for AAPL):

┌─────────────────────────────────────────────────────────┐
│ AAPL Price History (std::deque<PriceData>)             │
├─────────────────────────────────────────────────────────┤
│ [0] PriceData{$234.50, +0.23, t₀}                      │
│ [1] PriceData{$234.73, +0.15, t₁}                      │
│ [2] PriceData{$234.88, +0.45, t₂}                      │
│ ...                                                     │
│ [98] PriceData{$241.22, -0.31, t₉₈}                    │
│ [99] PriceData{$241.05, -0.17, t₉₉}  ← Most recent    │
└─────────────────────────────────────────────────────────┘
         ▲                         ▲
         │                         │
    Oldest (pop_front)      Newest (push_back)
    when size > 100
```

## Condition Variable Wait/Notify Pattern

```
Producer Thread:              Shared Buffer:              Consumer Thread:
                                                         
Generate data        ─────►  Mutex locked               
                             Add to buffer              Sleeping on CV
                             Mutex unlocked             (waiting for signal)
                             ▼                          
                             cv.notify_all() ─────────► Woken up!
                                                        Acquire mutex
                                                        Read data
                                                        Release mutex
                                                        Process data
                                                        ▼
Generate more data   ─────►  Mutex locked               Back to waiting
                             Add to buffer              
                             Mutex unlocked             
                             ▼                          
                             cv.notify_all() ─────────► Woken again!
                                                        (Cycle continues)
```

## Performance Measurement Flow

```
┌──────────────────────────────────────────────────────────┐
│              Producer: Generate Price                     │
│  timestamp_gen = high_resolution_clock::now()            │
│  ↓                                                        │
│  Store in PriceData.timestamp                            │
└────────────────────┬─────────────────────────────────────┘
                     │
                     ↓ (Data flows to buffer)
                     │
┌────────────────────┴─────────────────────────────────────┐
│           Consumer: Calculate Indicator                   │
│  timestamp_calc = high_resolution_clock::now()           │
│  latency = timestamp_calc - timestamp_gen                │
│  ↓                                                        │
│  Record in PerformanceMonitor                            │
└────────────────────┬─────────────────────────────────────┘
                     │
                     ↓
┌────────────────────┴─────────────────────────────────────┐
│         PerformanceMonitor: Aggregate Stats              │
│  • Min/Max/Avg latency per symbol                        │
│  • Throughput (ops/sec)                                  │
│  • System uptime                                         │
└──────────────────────────────────────────────────────────┘
```

## Thread Lifecycle

```
Main Thread Timeline:
═══════════════════════════════════════════════════════════════

│ Initialize   │ Start   │   Running...   │  Shutdown  │ Report │
│              │ Threads │                │            │        │
▼              ▼         ▼                ▼            ▼        ▼
Time ─────────────────────────────────────────────────────────►

Producer Thread:
           ┌──────────────────────────────────┐
           │ Generate prices in loop          │
           └──────────────────────────────────┘
           
Display Thread:
             ┌────────────────────────────────┐
             │ Display prices in loop         │
             └────────────────────────────────┘

SMA Thread:
               ┌──────────────────────────────┐
               │ Calculate SMA in loop        │
               └──────────────────────────────┘

Volatility Thread:
                 ┌────────────────────────────┐
                 │ Calculate volatility loop  │
                 └────────────────────────────┘
```

## Critical Section Analysis

```
Operation: SharedBuffer::push() (Producer)

┌─────────────────────────────────────────────────────────┐
│ OUTSIDE CRITICAL SECTION (Thread-safe)                  │
│ • Generate PriceData                                    │
│ • Create timestamp                                      │
└──────────────────────┬──────────────────────────────────┘
                       ▼
                  MUTEX LOCK
                       ▼
┌─────────────────────────────────────────────────────────┐
│ CRITICAL SECTION (Must be fast!)                        │
│ • price_history_[symbol].push_back(data)    ← ~10 ns   │
│ • if (size > max) pop_front()               ← ~10 ns   │
│ • ++total_writes_                           ← ~1 ns    │
│ TOTAL: ~21 nanoseconds                                  │
└──────────────────────┬──────────────────────────────────┘
                       ▼
                  MUTEX UNLOCK
                       ▼
┌─────────────────────────────────────────────────────────┐
│ OUTSIDE CRITICAL SECTION                                │
│ • cv.notify_all()                                       │
└─────────────────────────────────────────────────────────┘

Key: Minimal critical section → Low lock contention!
```

## Timing Diagram Example

```
Time (ms)   Producer        Display         SMA Calc       Volatility
───────────────────────────────────────────────────────────────────────
0           Generate ───┐
            AAPL $234   │
100         Generate    │   Read & ────┐
            AAPL $235   │   Display    │
200         Generate    │              │
            AAPL $236   │              │
...                     │              │
1000        Generate ───┴──────────────┴─► Calculate ──┐
            AAPL $241                      SMA = $238   │
1500        Generate                                    │
            AAPL $242                                   └─► Calculate
                                                             Vol = 2.3%
```

---

**Note**: All diagrams are conceptual representations to aid understanding of the multithreaded architecture and synchronization mechanisms.
