# Debug Cycle System Design

**Date:** 2024-12-28
**Status:** Implemented

## Problem Statement

During debugging sessions, the agent exhibits several failure patterns that waste hours:

1. **Call Graph Blindness** - Doesn't know what calls what, conflates unrelated functions
2. **Ownership Confusion** - Doesn't know which file owns which responsibility
3. **Magic Number Scattering** - Hardcodes values inline instead of using constants
4. **Guess-Then-Pray** - Adds fields hoping something works instead of reading working impl
5. **Error Conflation** - Treats different errors as the same problem
6. **Infrastructure Hallucination** - Invents wrong IPs, forgets sshpass, guesses paths
7. **Procedure Drift** - Improvises deployment instead of following exact commands
8. **State Loss** - Forgets what was tried across debug cycles

## Solution: Hook + Skill + State File Triad

### Components

| Component | Location | Purpose |
|-----------|----------|---------|
| Infrastructure Context | `~/.claude/debug-cycle/infrastructure.json` | IPs, passwords, exact commands |
| Codebase Map | `~/.claude/debug-cycle/codebase-map.md` | Call graph, file ownership, working impls |
| Debug State | `~/.claude/debug-cycle/debug-state.json` | Current phase, hypotheses, results |
| Skill Definition | `.claude/skills/debug-cycle/SKILL.md` | State machine + enforcement rules |
| Slash Command | `.claude/commands/debug.md` | Entry point to invoke skill |
| SessionStart Hook | `~/.claude/settings.json` | Auto-injects context on every session |

### Phase Machine

```
DIAGNOSE → FIX → BUILD → DEPLOY → VERIFY
    ↑                              ↓
    └──────── (on failure) ────────┘
```

Each phase has:
- Entry requirements (must complete previous phase)
- Mandatory checkpoints (cannot skip)
- Exit criteria (must meet to advance)
- State file update (persists progress)

### Key Enforcement Mechanisms

1. **SessionStart Hook** - Injects all context automatically; agent cannot avoid seeing it
2. **5-Gate Check** - Before any code change, must answer 5 specific questions
3. **Forbidden Patterns** - Inline magic numbers, parallel implementations auto-rejected
4. **Exact Deployment Commands** - Copied from infrastructure.json, no improvisation
5. **Hypothesis Tracking** - Every fix attempt logged with result
6. **3-Strike Rule** - After 3 failed hypotheses, must question architecture

## Files Created

```
~/.claude/debug-cycle/
├── infrastructure.json      # Static deployment constants
├── codebase-map.md          # Call graph, ownership, working impls
└── debug-state.json         # Current phase, hypothesis history

PROJECT-OMBRA/.claude/
├── skills/debug-cycle/
│   └── SKILL.md             # Full skill definition
└── commands/
    └── debug.md             # /debug slash command

~/.claude/settings.json      # Hook registration added
```

## Usage

### Starting a Debug Cycle

```
/debug
```

This will:
1. Read all context files
2. Check current state
3. Resume from current phase or start new cycle
4. Enforce the skill's requirements

### Manual Context Check

The SessionStart hook auto-injects context on every session start. To manually verify:

```bash
cat ~/.claude/debug-cycle/infrastructure.json
cat ~/.claude/debug-cycle/codebase-map.md
cat ~/.claude/debug-cycle/debug-state.json
```

### Updating Codebase Map

When architecture changes, update `~/.claude/debug-cycle/codebase-map.md` with:
- New call graph entries
- Updated file ownership
- New working implementations
- New error codes

## Success Metrics

- Zero improvised deployment commands (must copy from infrastructure.json)
- Zero inline magic numbers (must use supdrv_types.h defines)
- All code changes cite working implementation source
- Every hypothesis logged with tested/passed/failed status
- No more than 3 hypotheses per distinct error

## Failure Modes Prevented

| Before | After |
|--------|-------|
| "Let me try 192.168.1.100" | Uses 10.0.10.101 from infrastructure.json |
| "in.Hdr.fFlags = 0x42000042" | Uses SUPREQHDR_FLAGS_MAGIC constant |
| Modify driver_interface.c for IOCTL logic | Calls supdrv.c functions |
| "Session failed, let me try adding X" | Must identify exact rc value first |
| Guess path, get wrong path | Uses find command, exact path |
| ssh without sshpass → permission denied | Copies exact command from infrastructure.json |
