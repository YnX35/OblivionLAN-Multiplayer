# Development Guide

This document defines the technical baseline and working practices for contributors. It is intentionally conservative: exact toolchain versions must be verified before they are declared supported.

## Target environment

- Game: original PC release of *The Elder Scrolls IV: Oblivion*.
- Process architecture: 32-bit (x86).
- Extension framework: xOBSE.
- Initial network scope: local network only.
- Intended session model: one player hosts while remaining an active participant.
- Initial group-size objective: two to four players.

Oblivion Remastered is outside the scope of this project. Its engine and extension environment must not be treated as interchangeable with the original release.

## Toolchain selection

The project has not yet pinned a supported compiler, IDE, SDK revision or build system. Before implementation begins, contributors must:

1. Confirm the exact game executable and runtime.
2. Select an xOBSE release compatible with that runtime.
3. Obtain the matching SDK and inspect its headers and examples.
4. Select a compiler configuration that produces a 32-bit plugin.
5. Record all versions and required build settings in the repository.
6. Verify the resulting plugin in a clean test installation.

Do not invent API names or assume that a function from another xOBSE version is available.

## Repository structure

```text
/
├── README.md
├── CONTRIBUTING.md
├── SECURITY.md
└── docs/
    ├── ARCHITECTURE.md
    ├── DEVELOPMENT.md
    └── ROADMAP.md
```

Source, tests and build configuration should be added when their first implementation milestone is approved. Keep new directories focused and document their purpose.

## Implementation boundaries

- Keep protocol code independent from Oblivion-specific code.
- Do not call game-engine functions from networking workers.
- Route validated game operations through a bounded queue to a verified game-thread context.
- Treat network and persisted data as untrusted input.
- Use explicit serialization; never transmit raw C++ structures.
- Keep multiplayer-owned persistence separate from vanilla saves wherever practical.
- Make shutdown, disconnect and recovery paths part of the design, not later additions.

## Build and test reporting

Every build or test report should identify the tested revision and environment. At minimum, record:

- operating system;
- game version and executable architecture;
- xOBSE and SDK versions;
- compiler and build configuration;
- test procedure;
- observed result;
- known limitations.

Distinguish clearly between code that is designed, implemented, compiled, tested in isolation and validated in-game. These terms are not interchangeable.

## Local test data

Use dedicated test installations and disposable test saves. Do not use personal saves as the only validation environment. Never commit game assets, save files, binaries, memory dumps, credentials or private data.

## Release policy

No release should be published until there is a reproducible build procedure, a documented compatibility matrix, an installation and removal procedure, and test evidence for the advertised functionality.
