---
name: BurnoutRevenge Regression Lessons
description: What caused BurnoutRevenge (the prior game project) to fail — lessons to apply to OpenCity
type: project
---

BurnoutRevenge (/apps/git/BurnoutRevenge) was abandoned after persistent regressions. The project had a tiered test plan (TESTPLAN.md) and UE Automation Framework tests, but commits like "Fix city select blocking car input" and "Fix 3 rendering bugs: building heights, collision, material" show that new features routinely broke existing ones.

Root causes observed:
- Tests existed but were not enforced before merging new features
- Large feature commits introduced multiple regressions at once
- Some test logic was PIE-dependent, making headless CI impractical
- No pure-function Core layer — game logic was entangled with UObject, making unit tests fragile

**Why:** The user abandoned the project due to this cycle; they specifically requested extensive testing for OpenCity.
**How to apply:** Design a pure-function Core/ layer with zero UObject dependency so tests run headlessly. Write tests before features. Run the full automation suite after every significant change before committing. Enforce tier-blocking (Tier 1 must pass before Tier 2 work begins).
