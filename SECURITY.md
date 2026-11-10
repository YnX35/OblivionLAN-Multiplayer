# Security Policy

## Scope

OblivionLAN is intended to begin as a local-network multiplayer extension for the original PC release of *The Elder Scrolls IV: Oblivion*. The LAN scope does not make connected peers inherently trustworthy.

This policy applies to the project source, protocol, development tools and any future distributed builds.

## Security requirements

The implementation must:

- treat all network messages and persisted records as untrusted input;
- enforce strict limits on message sizes, field lengths, queue capacity and processing rates;
- validate message types, protocol versions, identifiers and numeric ranges;
- reject malformed, truncated, duplicate or incompatible data safely;
- avoid unbounded allocations and unsafe deserialization;
- never interpret network-provided data as executable code or an arbitrary filesystem path;
- never write to game files or save data based on an untrusted remote instruction;
- keep the initial service limited to the local network;
- provide controlled timeout, disconnect and shutdown behaviour;
- avoid requiring users to disable firewalls or endpoint-protection software;
- ensure diagnostics do not expose credentials, personal data or private save contents.

## Save-data protection

Testing must use dedicated copies of game installations and disposable saves. Changes that may affect vanilla save data require explicit backup and recovery procedures. No feature should be described as safe for personal saves until that claim has been validated.

## Supported versions

No official binary release or supported runtime matrix is currently published. Compatibility and security claims must identify the exact source revision, game version, xOBSE version and build configuration tested.

## Reporting a vulnerability

For a non-sensitive defect, open a GitHub issue with:

- a concise description;
- affected revision and environment;
- reproducible steps;
- relevant logs with private information removed.

Do not publish passwords, access tokens, private keys, personal save files, memory dumps or other sensitive data in an issue.

For a vulnerability that could affect users beyond the reporter, avoid posting exploit details publicly before maintainers have had a reasonable opportunity to assess and address the issue. Use GitHub's private vulnerability-reporting feature if it is enabled for the repository.

## Disclosure

The project is currently pre-implementation. A formal response-time commitment is not established. Reports will be assessed as project capacity permits, and any confirmed issue and remediation will be documented in the repository.
