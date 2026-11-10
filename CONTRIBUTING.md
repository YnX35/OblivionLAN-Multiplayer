# Contributing to OblivionLAN

Thank you for your interest in contributing. The project is at an early design stage, so clear proposals, careful technical investigation and reproducible tests are especially valuable.

## Before you start

1. Read the [project overview](README.md), [architecture](docs/ARCHITECTURE.md), [development guide](docs/DEVELOPMENT.md) and [roadmap](docs/ROADMAP.md).
2. Check existing issues and pull requests to avoid duplicate work.
3. For substantial changes, open an issue first and describe the problem, proposed approach and compatibility impact.
4. Verify game-engine and xOBSE APIs against the exact SDK and runtime version being targeted. Do not rely on undocumented assumptions.

## Areas for contribution

The project may need help with:

- xOBSE plugin development and reverse-engineering research;
- network protocol design and implementation;
- session management and host-authoritative state;
- safe game-thread integration;
- persistence, recovery and save compatibility;
- automated tests, test tooling and technical documentation.

The roadmap defines the current order of work. Please coordinate before beginning work on an area that is already in progress.

## Branches and pull requests

- Keep `main` stable.
- Create a focused branch for each change, using a descriptive name such as `feature/protocol-framing` or `fix/packet-validation`.
- Keep each pull request limited to one coherent change.
- Explain the motivation, design, compatibility impact and known limitations.
- Link related issues.
- Update documentation and tests when behaviour or design changes.
- Do not merge unreviewed changes that affect save data, protocol compatibility or game-engine hooks.

## Code quality

- Use clear, consistent names and document non-obvious invariants.
- Validate all external input, including network messages and persisted data.
- Bound allocations, queues and message sizes.
- Handle errors and disconnects explicitly.
- Avoid undefined behaviour, unchecked casts and assumptions about object lifetime.
- Keep networking code independent from direct game-engine calls.
- Justify new dependencies and record the exact version used.
- Do not add an API call unless it is supported by the selected SDK/runtime.

## Testing and reporting

For every change, state exactly which checks were performed. Relevant checks may include:

- static review;
- compilation, including compiler version and target architecture;
- unit or protocol tests;
- two-process or two-machine LAN tests;
- in-game tests using a documented game and xOBSE configuration;
- save, reload, recovery and disconnect tests.

Never describe a test as passed unless it was actually executed. If a test could not be run, say so and explain why.

## Files that must not be committed

Do not commit:

- game installation files or Bethesda-owned assets;
- personal save files or memory dumps;
- compiled binaries unless a release process explicitly requires them;
- credentials, access tokens, private keys or local configuration containing secrets;
- personal data or logs that expose private information.

## Pull request checklist

- [ ] The change has a clear, limited purpose.
- [ ] Relevant design and compatibility assumptions are documented.
- [ ] External inputs are validated and bounded.
- [ ] Errors, timeouts and disconnects are handled where relevant.
- [ ] Tests were run and their results are accurately reported.
- [ ] Documentation and roadmap status are up to date.
- [ ] No game assets, personal data, secrets or unintended binaries are included.
