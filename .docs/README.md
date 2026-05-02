# OpenCity Architecture Docs

This folder captures architecture and system decisions for OpenCity.

## Documents

- ADR-001: Layered Architecture and Data Ownership (`ADR-001-layered-architecture.md`)
- ADR-002: Runtime Data Flow (Spawn, Streaming, Interaction) (`ADR-002-runtime-data-flow.md`)
- ADR-003: UML Class Structure and Relationships (`ADR-003-uml-structure.md`)

## Scope

These docs describe current implementation decisions in `Source/OpenCity`:

- Core deterministic math/data layer
- Actor and subsystem wrappers around Core
- Input, possession, and movement flow
- Procedural city streaming and building spawn lifecycle

## Update Rule

When behavior changes, update the relevant ADR and add a short "Consequences" note.
