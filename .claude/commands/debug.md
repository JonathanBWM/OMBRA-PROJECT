---
description: Start or resume a debug cycle for OMBRA hypervisor loader
---

# Debug Cycle Command

You are entering a structured debug cycle. Read and follow the debug-cycle skill exactly.

## Step 1: Load Context

Read these files NOW before doing anything else:

1. `~/.claude/debug-cycle/infrastructure.json` - IPs, passwords, commands (DO NOT IMPROVISE)
2. `~/.claude/debug-cycle/codebase-map.md` - Call graph, file ownership, working implementations
3. `~/.claude/debug-cycle/debug-state.json` - Current phase and hypothesis history

## Step 2: Determine Current State

Check `debug-state.json`:
- If `phase` is "IDLE" → Start new cycle, transition to DIAGNOSE
- If `phase` is not "IDLE" → Resume from that phase

## Step 3: Follow the Skill

Invoke the debug-cycle skill and follow it EXACTLY:
- No skipping phases
- No improvising deployment commands
- No inline magic numbers
- No guessing without reading working implementations

## Step 4: Update State

After EVERY phase transition, update `debug-state.json` with:
- New phase
- Hypothesis status
- Error details
- Files modified

## Critical Rules

1. **NEVER type IPs from memory** - Copy from infrastructure.json
2. **NEVER skip the 5-gate check** in FIX phase
3. **NEVER guess protocol fields** - Copy from supdrv.c
4. **ALWAYS paste raw output** - Never summarize or interpret
5. **STOP after 3 failed hypotheses** - Question architecture

Begin by reading the three context files listed in Step 1.
