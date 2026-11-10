# Technical Architecture

**Document status:** proposed design. The components described here have not yet been implemented or validated in-game.

## 1. Scope and design objectives

The initial architecture targets the original 32-bit PC release of *The Elder Scrolls IV: Oblivion*, extended through xOBSE. The first networking milestone is a two-player LAN prototype, with a longer-term target of two to four players.

The design prioritizes:

- isolation between networking and game-engine operations;
- host-authoritative shared state;
- bounded resource usage and defensive input validation;
- minimal impact on vanilla save files;
- incremental implementation with independently testable components.

## 2. System overview

The proposed system has four logical components:

1. **Game integration:** an xOBSE plugin loaded into each game process.
2. **Session transport:** connection handling and message exchange over the LAN.
3. **Session authority:** host-side validation and coordination of shared state.
4. **Persistence:** storage and recovery of data owned by the multiplayer extension.

Conceptual topology:

```text
+-------------------+        LAN        +----------------------------+
| Player client A   |<----------------->|                            |
| Oblivion + plugin |                   | Host player                 |
+-------------------+                   | Oblivion + plugin            |
                                        | Session authority            |
+-------------------+        LAN        | Shared-state persistence     |
| Player client B   |<----------------->|                            |
| Oblivion + plugin |                   +----------------------------+
+-------------------+
```

The host is also an active player. A separate dedicated server is not required for the initial design.

## 3. Authority and state ownership

The host is the authority for shared multiplayer state. Clients submit requests or player actions; they do not directly overwrite authoritative state.

Candidate host-owned data includes:

- session membership and player identifiers;
- supported shared interactions;
- explicitly supported quest progression;
- entities created or managed by the multiplayer extension;
- persistent multiplayer-world records.

The exact ownership rules and conflict-resolution model must be specified before implementation.

### Quest and encounter boundaries

Quest scripts in the original game may trigger local side effects, alter persistent records or depend on player-specific state. General quest synchronization must therefore not be assumed.

The implementation should begin with a deliberately limited set of reversible interactions. Ordinary encounters outside supported shared quest scenarios should remain independent unless a specific feature requires otherwise. Unique actors and quest-critical entities must be excluded from generic respawn or cleanup rules.

## 4. Game-thread boundary

Networking code must never call internal game-engine functions directly.

Proposed processing flow:

1. Receive network bytes on a networking worker.
2. Enforce framing and maximum message size.
3. Decode fields using explicit byte-order and type conversions.
4. Validate protocol version, message type, identifiers and value ranges.
5. Place validated commands into a bounded queue.
6. Process game-related operations from a verified safe game-thread context.
7. Return the host-approved result to connected peers.

Queue capacity, overflow policy, thread ownership and shutdown ordering must be defined and tested. Queue saturation must produce a controlled failure, not unbounded memory growth.

## 5. Network protocol

No transport or serialization format has been selected. A protocol specification must define:

- protocol identifier and version;
- message type and framing;
- strict maximum message and field lengths;
- session and peer identifiers;
- sequence numbers and duplicate handling;
- explicit integer representation and byte order;
- handling of unknown, malformed, truncated and out-of-order messages;
- connection timeout, heartbeat and clean shutdown behaviour.

Raw C++ structures must not be transmitted directly. Padding, alignment and representation are not stable wire formats. A peer-provided length must never be trusted without validation.

TCP, UDP or a hybrid approach remains an open decision. It should be selected after a standalone prototype and measurements on a local network.

## 6. World and entity identity

Vanilla FormIDs must not be assumed to be globally stable across installations, load orders or mod configurations.

A multiplayer identity scheme must be designed for each supported entity category. Candidate categories include:

- connected player;
- extension-managed ordinary creature;
- unique or quest-critical actor;
- shared interactive object;
- explicitly supported quest state.

Unknown or unclassified entities must be left untouched by default. The extension must not generically delete, reset or respawn entities it cannot safely identify.

## 7. Persistence and recovery

Multiplayer-owned data should be stored separately from vanilla save data wherever practical. SQLite is a candidate, not a final decision.

The persistence design must address:

- schema versioning and migrations;
- atomic writes and transaction boundaries;
- world and session identity;
- backup and restore;
- missing, locked or corrupted storage;
- interrupted writes and unexpected process termination;
- host recovery and client reconnection.

Only the host may commit authoritative shared-state changes. Clients must not be able to overwrite host-owned records.

## 8. Security and resilience

A local network is not a trusted boundary. Every message must be treated as untrusted input.

The implementation must:

- cap message sizes, queue lengths and processing rates;
- reject malformed, non-finite and out-of-range values;
- handle incompatible protocol versions explicitly;
- avoid interpreting network data as executable code or file paths;
- never write to game files based on a remote-supplied path;
- keep the initial service LAN-scoped;
- avoid requiring users to disable their firewall or security software;
- log useful diagnostics without exposing secrets or personal data.

See [SECURITY.md](../SECURITY.md) for the project security policy.

## 9. Open technical decisions

The following decisions require prototypes or verified documentation:

| Area | Current status |
|---|---|
| Exact Oblivion runtime | To be confirmed and recorded |
| xOBSE release and SDK | To be selected and pinned |
| Compiler and build system | To be selected for the x86 target |
| Transport | Open |
| Serialization format | Open |
| Threading and queue implementation | Open |
| Shared entity identity | Open |
| Persistence backend | Open |
| Supported quest scope | Open |

No compatibility or performance claim should be made until the relevant decision has been implemented and tested.
