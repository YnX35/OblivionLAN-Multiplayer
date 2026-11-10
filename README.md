# OblivionLAN

**A local-network multiplayer project for the original 2006 release of The Elder Scrolls IV: Oblivion.**

OblivionLAN is an independent community project exploring a cooperative multiplayer experience for the original Gamebryo-based PC release of *Oblivion*. The long-term objective is to let a small group of players share a session hosted by one of the players, while preserving the behaviour and stability of the original game.

> **Development status: pre-implementation.** The repository currently contains project documentation only. It does not yet provide a playable plugin, network client, host service, installer or compiled release. No multiplayer feature is currently claimed to work.

## Project goals

The initial target is a **2–4 player LAN session** with a player-hosted world.

Planned capabilities:

- **Player presence:** connect multiple game instances and represent remote players in the same world.
- **World-state authority:** let the host validate changes to shared state.
- **Shared progression:** support selected cooperative quest and world interactions, introduced incrementally.
- **Persistence:** retain mod-managed shared data between sessions.
- **Independent ordinary encounters:** avoid unnecessarily coupling routine encounters that are not part of a supported shared quest.
- **Safe failure handling:** handle disconnects, incompatible protocol versions and invalid network data without corrupting saves.

These are project objectives, not implemented features.

## Compatibility target

| Component | Initial target |
|---|---|
| Game | The original PC release of *The Elder Scrolls IV: Oblivion* (2006) |
| Game edition | Classic release; not Oblivion Remastered |
| Game process | 32-bit |
| Extension framework | xOBSE |
| Operating system | Windows |
| Network scope | Local network (LAN) |
| Session model | Player-hosted, host-authoritative |
| Intended group size | 2–4 players |

Exact supported game, xOBSE, SDK and compiler versions have not yet been frozen. Compatibility will only be claimed after reproducible testing against a documented configuration.

## Current repository contents

This repository is currently a **design and project-planning repository**. It contains:

- [Architecture](docs/ARCHITECTURE.md) — proposed component boundaries and technical decisions.
- [Roadmap](docs/ROADMAP.md) — implementation phases and acceptance criteria.
- [Development guide](docs/DEVELOPMENT.md) — development constraints and validation expectations.
- [Contributing](CONTRIBUTING.md) — contribution workflow and review requirements.
- [Security policy](SECURITY.md) — network and data-handling requirements.

There is no source implementation or downloadable build at this stage.

## Proposed architecture

The design separates the game integration from networking:

1. An xOBSE plugin runs inside each Oblivion process.
2. A networking layer exchanges versioned messages over the local network.
3. The host validates requests and owns the authoritative shared state.
4. Game-engine operations are queued and processed on an appropriate game thread; networking code must not call the game engine directly.
5. Mod-managed persistent data is stored separately from vanilla save data wherever practical.

The architecture is a proposal. Transport, serialization, persistence and runtime integration decisions remain open until validated by prototypes.

## Development principles

- Protect vanilla saves and provide a recoverable failure path.
- Treat every network message as untrusted input.
- Keep the initial release LAN-only.
- Do not claim compatibility or functionality without reproducible tests.
- Record exact toolchain and runtime versions for every validated build.
- Prefer small, reviewable changes over broad unverified modifications.

## Getting involved

Contributions are welcome in systems engineering, C++/xOBSE development, networking, testing and technical documentation.

Before proposing implementation work, please read [CONTRIBUTING.md](CONTRIBUTING.md), [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and [docs/ROADMAP.md](docs/ROADMAP.md). Open an issue to discuss substantial changes before starting work.

## Project status and releases

There are currently no official releases. A release will only be published after its supported configuration, installation procedure and test results are documented.

## Disclaimer

OblivionLAN is an independent, community-developed project. It is not affiliated with, endorsed by or sponsored by Bethesda Softworks or its affiliates. *The Elder Scrolls* and *Oblivion* are trademarks of their respective owners.
