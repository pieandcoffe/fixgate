# fixgate

**A low-latency FIX 4.4 order gateway written in modern C++20 for Linux.**

> 🚧 **Status: in development.** This repository describes the planned design and roadmap. Code is pushed milestone by milestone; the checklist below shows what's finished.

fixgate accepts FIX connections from trading clients, runs the full FIX session layer (logon, heartbeats, sequence numbers, gap recovery), tracks every order through its lifecycle in an order management layer with pre-trade risk checks, matches orders in an in-memory order book with price-time priority, and returns execution reports. The goal is a small but realistic model of a broker's client connectivity gateway, built with a focus on correctness, measurable latency and clean engineering practices.

---

## Features (planned)

### FIX protocol
- FIX 4.4 tag=value codec: TCP stream framing, zero-copy parsing into `std::string_view`, serializer that computes BodyLength (9) and CheckSum (10)
- Session layer: Logon/Logout, Heartbeat and TestRequest timers, MsgSeqNum validation, ResendRequest handling, SequenceReset-GapFill, PossDupFlag processing
- Sequence numbers and sent messages persisted to an append-only journal, so a restarted gateway resumes the session without gaps

### Order management
- Messages: NewOrderSingle (D), OrderCancelRequest (F), OrderCancelReplaceRequest (G), ExecutionReport (8), OrderCancelReject (9)
- Order state machine: PendingNew → New → PartiallyFilled → Filled, plus PendingCancel/Canceled, PendingReplace/Replaced and Rejected
- ClOrdID / OrigClOrdID chain tracking, duplicate order detection, "too late to cancel" handling
- Pre-trade risk checks: maximum quantity, maximum notional, price collars, restricted symbols
- Invariants enforced and tested: `CumQty + LeavesQty == OrderQty`, no overfills

### Matching engine
- Limit order book per symbol with price-time (FIFO) priority
- Order types: Limit and Market; time in force: Day, GTC, IOC, FOK
- Two book implementations benchmarked against each other: `std::map`-based and flat array per price tick

### Performance
- Thread-per-stage pipeline: network thread ↔ engine thread connected by lock-free single-producer/single-consumer ring buffers
- No heap allocation on the hot path after warm-up
- Non-blocking TCP with `epoll`, `TCP_NODELAY`
- Integer fixed-point prices
- Latency histograms (p50 / p99 / p99.9 / max) measured from message receive to response send

---

## Architecture

```
            TCP                                          TCP
 Clients ─────────►┌────────────────────┐          ┌──────────────► Clients
                   │  Network thread    │          │
                   │  epoll → framing → │          │
                   │  codec → session   │          │
                   └─────────┬──────────┘          │
                             │ SPSC queue          │
                   ┌─────────▼──────────┐          │
                   │  Engine thread     │          │
                   │  validation + risk │          │
                   │  order state       │          │
                   │  matching engine   │          │
                   └─────────┬──────────┘          │
                             │ SPSC queue          │
                   ┌─────────▼──────────┐          │
                   │  Network thread    ├──────────┘
                   │  encode + send     │
                   └─────────┬──────────┘
                             ▼
                   Journal (sequence numbers + messages)
```

### Design decisions
| Decision | Reason |
|---|---|
| One thread per stage, SPSC queues between them | No locks on the hot path; each stage owns its data |
| `std::string_view` fields into the receive buffer | Zero-copy parsing; buffer lifetime managed explicitly |
| Concepts instead of virtual interfaces | Static dispatch, inlinable calls, readable compile errors |
| Integer ticks for prices | Exact decimal arithmetic, no floating-point drift |
| Append-only journal | Simple, fast recovery of sequence numbers after a restart |
| `steady_clock` for timers and latency | Monotonic; unaffected by wall-clock adjustments |

---

## Tech stack

| Area | Tools |
|---|---|
| Language | C++20 (concepts, `std::span`, `std::jthread`, `std::stop_token`, `<bit>`, designated initializers) |
| Platform | Linux, POSIX sockets, `epoll` |
| Build | CMake (presets), Conan |
| Testing | GoogleTest, integration tests with a Python FIX client |
| Benchmarking | Google Benchmark, custom latency histogram, `perf` |
| Quality | AddressSanitizer, UndefinedBehaviorSanitizer, ThreadSanitizer, clang-tidy, clang-format |
| CI/CD | GitHub Actions (gcc + clang, sanitizer builds) |
| Packaging | Docker (multi-stage build) |
| Scripting | Python (test client, load generator), Bash (scenario runner) |

---

## Repository layout

```
fixgate/
├── CMakeLists.txt
├── CMakePresets.json        # debug, release, asan, tsan
├── conanfile.txt
├── include/fixgate/
│   ├── codec/               # framing, parser, serializer, tags
│   ├── session/             # session state machine, timers, sequence store
│   ├── oms/                 # orders, state machine, risk checks
│   ├── engine/              # order book, matching
│   ├── net/                 # epoll reactor, TCP connections
│   └── util/                # SPSC queue, clocks, latency histogram
├── src/
├── tests/
├── bench/
├── tools/
│   ├── client.py            # FIX test client and load generator
│   └── run_scenario.sh      # build, start gateway, run a trading scenario
├── docker/Dockerfile
└── .github/workflows/ci.yml
```

---

## Roadmap

- [ ] **M0: Skeleton and CI.** CMake presets, Conan, GoogleTest, GitHub Actions with sanitizer jobs
- [ ] **M1: FIX codec.** Framing, zero-copy parser, checksum/BodyLength validation, serializer, typed message views, codec benchmark
- [ ] **M2: TCP transport.** Non-blocking `epoll` server, read/write buffering, Python test client
- [ ] **M3: Session layer.** Logon, heartbeats, TestRequest, sequence validation, resend and gap fill, logout
- [ ] **M4: Order management.** Order store, ClOrdID chains, risk checks, order state machine, execution reports
- [ ] **M5: Matching engine.** Price-time priority book, Limit/Market, IOC/FOK, map vs flat-array benchmark
- [ ] **M6: Threading pipeline.** SPSC queues, `std::jthread` stages, mutex-queue baseline for comparison
- [ ] **M7: Persistence and recovery.** Journal, restart with sequence recovery, kill -9 recovery test
- [ ] **M8: Benchmarks and profiling.** Latency percentiles, throughput under load, `perf` analysis, results table
- [ ] **M9: Packaging.** Docker image, scenario script, optional interop test against QuickFIX/J

---

## Building (once M0 lands)

```bash
# Requirements: Linux, gcc 13+ or clang 17+, CMake 3.25+, Conan 2
conan install . --output-folder=build --build=missing
cmake --preset release
cmake --build --preset release
ctest --preset release

# Sanitizer builds
cmake --preset tsan && cmake --build --preset tsan && ctest --preset tsan
```

## Running (planned)

```bash
./build/fixgate --port 9878 --sender-comp-id GATEWAY
python3 tools/client.py --host 127.0.0.1 --port 9878 --orders 1000
```

---

## Benchmark results

To be published after M8, with CPU model, compiler, build flags and thread pinning documented for every number.

| Metric | Result |
|---|---|
| Codec parse throughput | TBD |
| Receive → response latency (p50 / p99 / p99.9) | TBD |
| Sustained orders per second | TBD |

---

## References

- [FIX Trading Community – standards](https://www.fixtrading.org/standards/)
- [FIXimate – FIX dictionary](https://fiximate.fixtrading.org/)
- [QuickFIX](https://www.quickfixengine.org/) (used as an interoperability counterparty)

## License

MIT
