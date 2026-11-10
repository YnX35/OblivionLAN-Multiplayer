# Development Roadmap

This roadmap defines the intended implementation order. A milestone is complete only when its acceptance criteria have been demonstrated and the evidence has been recorded.

## Current project status

**Pre-implementation — documentation only.**

The repository does not currently contain a playable plugin, network implementation, installer or release build. The milestones below are planned work, not completed functionality.

## Milestone overview

| Milestone | Objective | Status |
|---|---|---|
| 0 | Establish a clean game baseline | Not started |
| 1 | Build and load a minimal xOBSE plugin | Not started |
| 2 | Specify and test a standalone network protocol | Not started |
| 3 | Establish a two-peer LAN session | Not started |
| 4 | Synchronize limited in-game player state | Not started |
| 5 | Persist a limited shared-world interaction | Not started |
| 6 | Evaluate ordinary creatures and selected quest interactions | Not started |

## Milestone 0 — Game baseline

**Objective:** establish a reproducible, unmodified test installation.

Acceptance criteria:

- [ ] The original game launches without the extension.
- [ ] A new test game can be started.
- [ ] Save and reload complete successfully.
- [ ] The executable version and relevant file hashes are recorded.
- [ ] Test saves are kept separate from personal saves.
- [ ] The operating system and test hardware are documented.

## Milestone 1 — Minimal xOBSE plugin

**Objective:** prove that the selected build environment and plugin-loading path work.

Acceptance criteria:

- [ ] The target architecture is confirmed as x86/Win32.
- [ ] Exact game, xOBSE, SDK and compiler versions are recorded.
- [ ] The plugin uses verified interfaces from the selected SDK.
- [ ] Initialization and shutdown complete without a crash.
- [ ] A minimal diagnostic confirms plugin loading.
- [ ] The plugin is tested in a clean test installation.

No networking or world-entity manipulation is included in this milestone.

## Milestone 2 — Standalone protocol

**Objective:** implement and test the wire protocol independently of Oblivion.

Acceptance criteria:

- [ ] Message framing and versioning are documented.
- [ ] Field representation and byte order are explicit.
- [ ] Message and queue limits are enforced.
- [ ] Truncated, oversized, unknown and malformed messages are tested.
- [ ] Invalid input is rejected without out-of-bounds access or unbounded allocation.
- [ ] A reproducible local client/server test is available.

## Milestone 3 — Two-peer LAN session

**Objective:** establish a session between two peers on the same local network.

Acceptance criteria:

- [ ] Two machines or processes can establish a LAN connection.
- [ ] The host validates session requests.
- [ ] Peer identity and protocol compatibility are checked.
- [ ] Timeouts and clean disconnects are handled.
- [ ] Network workers do not call the game engine.
- [ ] The test does not require exposing a service to the public Internet.

## Milestone 4 — In-game player synchronization

**Objective:** synchronize a small, reversible subset of player state.

Acceptance criteria:

- [ ] Two players can be represented in a controlled test area.
- [ ] Position update frequency and network load are measured.
- [ ] Interpolation and correction behaviour are documented.
- [ ] Cell transitions are tested separately.
- [ ] Disconnects and malformed updates do not crash the game.
- [ ] No vanilla save corruption is observed in the defined test cases.

## Milestone 5 — Persistent shared-world interaction

**Objective:** share one limited world interaction and preserve its state across sessions.

Acceptance criteria:

- [ ] The host owns the authoritative record.
- [ ] Data is versioned and validated.
- [ ] Atomic writes and backups are tested.
- [ ] Restart, reload and reconnect behaviour is consistent.
- [ ] Unknown and unique entities are protected by default.
- [ ] Recovery behaviour is documented.

## Milestone 6 — Creatures and selected quest interactions

**Objective:** expand synchronization without compromising quest state or ordinary gameplay.

Implementation order:

1. Identify and test extension-managed ordinary creatures.
2. Define safe respawn behaviour for those creatures only.
3. Select a small set of quest interactions for isolated testing.
4. Test conflicts, disconnects, save/reload and recovery.
5. Document unsupported quest and mod combinations.

Unique actors and quest-critical entities must never be included in generic respawn rules. Full synchronization of the vanilla quest system is not promised.

## Validation record

For each completed milestone, record:

- date and milestone identifier;
- game, xOBSE, SDK and compiler versions;
- operating system and test configuration;
- exact test procedure and commands;
- observed result and relevant sanitized logs;
- known limitations and unresolved defects.

A milestone must not be marked complete based solely on design review or code inspection.
