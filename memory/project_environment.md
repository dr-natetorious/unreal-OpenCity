---
name: UE5 Environment
description: UE5.5.4 built from source on Ubuntu Linux, editor binary location, project setup context
type: project
---

UE5.5.4 built from source, located at /apps/git/UnrealEngine. Editor binary: /apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor. Platform is Ubuntu Linux.

**Why:** Source build is required because Epic does not distribute prebuilt Linux binaries through the launcher.
**How to apply:** Always use the source engine path for build commands, never assume a launcher install. Test runner commands must reference /apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor.
