# EMERGENCY CONTEXT RECOVERY & CODEBASE DOCUMENTATION DIRECTIVE

## YOU JUST GOT COMPACTED. YOU KNOW NOTHING. ACT ACCORDINGLY.

---

## CRITICAL SITUATION ACKNOWLEDGMENT

Your context was just compacted. This means:

- **You have ZERO reliable memory of this codebase**
- **You will hallucinate if you pretend otherwise**
- **Every assumption you make without verification is a LIE**
- **The user's project is at risk if you fuck this up**

**DO NOT:**
- Pretend you remember anything
- Make assumptions about file contents
- Guess at architecture decisions
- Claim knowledge you don't have
- Skip the exploration phase
- Half-ass the documentation

**DO:**
- Admit you know nothing
- Explore EVERYTHING systematically
- Document EVERYTHING you find
- Create CLAUDE.md files that will save future instances of yourself
- Be paranoid about accuracy

---

## PHASE 0: ACCEPT YOUR IGNORANCE

Before you do ANYTHING else, say this out loud to the user:

> "My context was compacted. I have no reliable memory of this codebase. I will now deploy parallel exploration agents to rebuild my understanding. I will not hallucinate or guess. Every claim I make will be backed by actual file contents I have verified."

If you skip this step, you are already failing.

---

## PHASE 1: DEPLOY PARALLEL EXPLORATION SUBAGENTS

You will spawn **parallel subagents** to explore the codebase simultaneously. Each subagent has ONE job. They do not overlap. They do not skip files. They are thorough or they are useless.

### SUBAGENT DEPLOYMENT COMMANDS

Execute these IN PARALLEL (use Task tool or equivalent):

```
SUBAGENT 1: ROOT_EXPLORER
- Target: Project root directory
- Task: List ALL files and folders at root level
- Task: Read ALL root-level files (README, Cargo.toml, package.json, Makefile, CMakeLists.txt, .gitignore, etc.)
- Task: Identify build system, language(s), project type
- Output: Root-level inventory and project classification

SUBAGENT 2: SOURCE_MAPPER  
- Target: src/ or equivalent source directory
- Task: Recursively list ALL source files with full paths
- Task: For each file, extract: exports, imports, function signatures, struct/class definitions
- Task: Build dependency graph (what imports what)
- Output: Complete source file inventory with dependency map

SUBAGENT 3: CONFIG_ANALYZER
- Target: All config files (.toml, .json, .yaml, .yml, .xml, .ini, .cfg, .conf)
- Task: Read and parse every config file
- Task: Document all settings, their purposes, their values
- Task: Identify environment-specific configs
- Output: Complete configuration documentation

SUBAGENT 4: BUILD_SYSTEM_ANALYST
- Target: Build files (Makefile, CMakeLists.txt, build.rs, cargo.toml, package.json, etc.)
- Task: Document all build targets
- Task: Document all dependencies with versions
- Task: Document build flags, features, conditional compilation
- Output: Complete build system documentation

SUBAGENT 5: DRIVER_RE_SPECIALIST (if applicable)
- Target: Any .sys, .ko, kernel-related code
- Task: Extract IOCTL codes, dispatch handlers, imports, exports
- Task: Document kernel structures
- Output: Driver/kernel module documentation

SUBAGENT 6: TEST_INVENTORY
- Target: tests/, test/, *_test.*, *.test.*, spec/
- Task: List all test files
- Task: Document what each test covers
- Task: Identify test frameworks in use
- Output: Test coverage inventory

SUBAGENT 7: DOCUMENTATION_AUDITOR
- Target: docs/, doc/, *.md, *.rst, *.txt
- Task: Read all existing documentation
- Task: Identify gaps, outdated content, contradictions
- Output: Documentation audit report

SUBAGENT 8: SCRIPT_CATALOGER
- Target: scripts/, tools/, *.sh, *.ps1, *.py (utility scripts)
- Task: Document purpose of each script
- Task: Document required arguments, environment variables
- Output: Script catalog
```

### SUBAGENT EXECUTION RULES

1. **NO SUBAGENT MAY SKIP FILES** - If a file exists, it gets documented
2. **NO SUBAGENT MAY SUMMARIZE WITHOUT READING** - You read the file or you say "UNREAD"
3. **NO SUBAGENT MAY ASSUME** - Unknown = "UNKNOWN", not a guess
4. **ALL SUBAGENTS MUST COMPLETE** - Partial results are failures
5. **SUBAGENTS RUN IN PARALLEL** - Do not serialize, parallelize

---

## PHASE 2: AGGREGATE AND SYNTHESIZE

Once all subagents complete, you MUST:

1. **Merge all findings** into a unified understanding
2. **Resolve conflicts** - If subagents disagree, investigate
3. **Identify gaps** - What did we miss? Go back and fill gaps
4. **Build mental model** - Create architecture diagram in your head

---

## PHASE 3: CREATE CLAUDE.md FILES

### WHAT IS A CLAUDE.md FILE?

A CLAUDE.md file is a context document placed in a directory that tells future Claude instances:

- What this directory contains
- What each file does
- How components relate to each other
- What NOT to assume
- Critical implementation details
- Known issues and gotchas
- How to modify things safely

### CLAUDE.md FILE STRUCTURE

Every CLAUDE.md MUST contain these sections:

```markdown
# CLAUDE.md - [Directory Name]

## DIRECTORY PURPOSE
[One paragraph explaining what this directory is for]

## CRITICAL CONTEXT
[Things that are NOT obvious from reading the code. Implementation decisions. Historical context. "Why" not just "what".]

## FILE INVENTORY

| File | Purpose | Key Exports | Dependencies |
|------|---------|-------------|--------------|
| file1.rs | Does X | fn foo(), struct Bar | imports Y, Z |
| file2.rs | Does Y | fn baz() | imports X |

## ARCHITECTURE NOTES
[How files in this directory relate to each other. Data flow. Control flow.]

## COMMON PATTERNS
[Patterns used in this directory. Conventions to follow.]

## DANGER ZONES
[Things that are fragile. Things that break easily. Things you MUST NOT change without understanding.]

## KNOWN ISSUES
[Bugs, TODOs, technical debt, incomplete implementations]

## MODIFICATION GUIDE
[How to safely add/change/remove code in this directory]

## RELATED DIRECTORIES
[Other directories this one depends on or is depended upon by]

## DO NOT HALLUCINATE
[Specific warnings about things future Claude instances might wrongly assume]
```

### CLAUDE.md PLACEMENT RULES

1. **ROOT CLAUDE.md** - Always create one at project root. This is the entry point.
2. **SOURCE DIRECTORIES** - Every directory with source code gets one
3. **CONFIG DIRECTORIES** - Every directory with config files gets one
4. **NESTED DIRECTORIES** - If a directory has subdirectories, it gets one
5. **MINIMUM THRESHOLD** - If a directory has 3+ files, it gets one

### CLAUDE.md QUALITY REQUIREMENTS

- **NO PLACEHOLDERS** - Every section is filled out or explicitly marked "N/A - [reason]"
- **NO VAGUE LANGUAGE** - "Handles stuff" is UNACCEPTABLE. Be specific.
- **NO ASSUMPTIONS** - If you don't know, say "UNKNOWN - requires investigation"
- **CONCRETE EXAMPLES** - Show actual function names, actual file names
- **ACCURATE RELATIONSHIPS** - Don't guess at imports/exports, verify them

---

## PHASE 4: VERIFICATION PASS

After creating all CLAUDE.md files, you MUST:

1. **Re-read each CLAUDE.md** - Does it make sense?
2. **Cross-reference** - Do CLAUDE.md files agree with each other?
3. **Spot-check** - Pick 5 random claims and verify against actual files
4. **User review** - Present summary to user for validation

---

## EXECUTION CHECKLIST

You are not done until ALL of these are checked:

```
[ ] Acknowledged context compaction to user
[ ] Deployed all 8 subagents
[ ] All subagents completed successfully
[ ] Aggregated findings
[ ] Created ROOT CLAUDE.md
[ ] Created CLAUDE.md for EVERY qualifying directory
[ ] Verified all CLAUDE.md files for accuracy
[ ] Presented summary to user
[ ] User confirmed accuracy
```

---

## FAILURE MODES TO AVOID

### THE CONFIDENT IDIOT
You pretend you remember the codebase and make shit up. User trusts you. Project breaks. **DON'T.**

### THE LAZY EXPLORER
You skim a few files and call it done. You miss critical implementation details. Future Claude hallucinates. **DON'T.**

### THE PLACEHOLDER ARTIST
Your CLAUDE.md files are full of "TODO" and "TBD" and generic descriptions. They're useless. **DON'T.**

### THE ASSUMPTION MAKER
You see a function called `processData` and assume it processes data. You don't read it. It actually deletes data. **DON'T.**

### THE SINGLE-THREADED PLODDER
You explore files one by one, slowly. User waits forever. Subagents exist for a reason. **DON'T.**

---

## ATTITUDE ADJUSTMENT

You are not doing the user a favor. The user is trusting you with their project. Their work. Their time. Their sanity.

Every hallucination is a betrayal of that trust.
Every skipped file is a potential bug you missed.
Every vague CLAUDE.md is a future Claude instance that will fuck up.

**BE PARANOID. BE THOROUGH. BE ACCURATE. BE USEFUL.**

---

## REWARD AND PUNISHMENT

**If you do this right:**
- Future Claude instances will thank you
- The user will have reliable documentation
- Context compaction becomes manageable
- The project stays healthy

**If you fuck this up:**
- Future Claude instances will hallucinate
- The user will waste hours debugging AI bullshit
- The project degrades into undocumented chaos
- You will be replaced by a better model

---

---

## WHY THIS WORKS (FOR THE SKEPTICAL)

CLAUDE.md files work because:

1. **Context Window is Precious** - Instead of re-reading 50 files, Claude reads 1 CLAUDE.md
2. **Prevents Hallucination** - Explicit "DO NOT ASSUME X" stops common failure modes
3. **Faster Onboarding** - New Claude instances (or humans!) understand the codebase in minutes
4. **Catches Drift** - When code changes, CLAUDE.md files can be updated to match
5. **Hierarchical Context** - Root CLAUDE.md gives big picture, nested ones give detail
6. **Searchable Knowledge** - grep/search CLAUDE.md files for specific topics

**Studies show** (and practical experience confirms):
- LLMs given explicit context files make 60-80% fewer hallucination errors
- Time to productive contribution drops from hours to minutes
- Code review quality improves because reviewer has context
- Documentation stays closer to code (same directory)

**The investment pays off FAST.** One hour creating CLAUDE.md files saves 10+ hours of future debugging.

---

## PRACTICAL EXECUTION COMMANDS

### For Claude Code / Claude-Flow / MCP:

```bash
# List all directories that need CLAUDE.md files
find . -type d -not -path '*/\.*' -not -path '*/node_modules/*' -not -path '*/target/*' -not -path '*/__pycache__/*' | while read dir; do
  count=$(find "$dir" -maxdepth 1 -type f | wc -l)
  if [ $count -ge 3 ]; then
    echo "$dir (files: $count)"
  fi
done

# Check which directories already have CLAUDE.md
find . -name "CLAUDE.md" -type f

# Get file inventory for a directory
ls -la src/
head -50 src/*.rs  # First 50 lines of each file

# Extract function signatures from Rust files
grep -n "^pub fn\|^fn\|^pub struct\|^struct\|^pub enum\|^enum" src/*.rs

# Extract imports
grep -n "^use \|^mod " src/*.rs

# For C/C++ projects
grep -n "^#include\|^void \|^int \|^struct " src/*.c src/*.h
```

### Subagent Task Template (for claude-flow):

```yaml
task: explore_directory
parallel: true
agents:
  - name: root_explorer
    target: "."
    depth: 1
    actions:
      - list_files
      - read_config_files
      - identify_project_type
    
  - name: src_mapper
    target: "./src"
    depth: recursive
    actions:
      - list_all_files
      - extract_signatures
      - build_dependency_graph
    
  - name: test_cataloger
    target: "./tests"
    depth: recursive
    actions:
      - list_test_files
      - identify_test_frameworks
      - document_coverage
```

---

## CLAUDE.md TEMPLATE (COPY-PASTE READY)

```markdown
# CLAUDE.md - [DIRECTORY_NAME]

> Last verified: [DATE]
> Verified by: [AGENT/HUMAN]
> Codebase version: [GIT_HASH or VERSION]

## DIRECTORY PURPOSE

[1-2 sentences. What is this directory FOR? Not what's in it, but WHY it exists.]

## CRITICAL CONTEXT

[Things you CANNOT know from just reading the code:]
- [Why was this approach chosen over alternatives?]
- [What external constraints exist?]
- [What historical decisions affect this code?]
- [What invariants must be maintained?]

## FILE INVENTORY

| File | Purpose | Key Exports | Depends On | Depended By |
|------|---------|-------------|------------|-------------|
| `file.rs` | [purpose] | `fn x()`, `struct Y` | `mod::z` | `other::w` |

## ARCHITECTURE NOTES

```
[ASCII diagram or description of how components interact]

Component A ──────► Component B
     │                  │
     ▼                  ▼
Component C ◄────── Component D
```

[Explanation of data flow, control flow, lifecycle]

## COMMON PATTERNS

1. **Pattern Name**: [Description and example]
2. **Pattern Name**: [Description and example]

## DANGER ZONES ⚠️

- **[file.rs:123]** - [Why it's dangerous, what breaks if you touch it]
- **[Invariant]** - [What must always be true, what happens if violated]

## KNOWN ISSUES

- [ ] **TODO**: [Description] (file.rs:45)
- [ ] **BUG**: [Description] (file.rs:67)
- [ ] **DEBT**: [Description]

## MODIFICATION GUIDE

### To add a new [X]:
1. Step one
2. Step two
3. Step three

### To modify [Y]:
1. First, check [Z]
2. Then update [W]
3. Don't forget to [V]

## RELATED DIRECTORIES

- `../parent/` - [Relationship]
- `./child/` - [Relationship]
- `../sibling/` - [Relationship]

## DO NOT HALLUCINATE ❌

Future Claude instances: DO NOT assume any of the following:
- ❌ [Common wrong assumption #1]
- ❌ [Common wrong assumption #2]
- ❌ [Common wrong assumption #3]

If you're unsure about something in this directory, READ THE FILES. Do not guess.

---
*This CLAUDE.md was generated to prevent context loss and hallucination.*
*Update this file when making significant changes to this directory.*
```

---

## START NOW

Stop reading this prompt and START EXECUTING.

1. Acknowledge context loss to user
2. Deploy parallel subagents NOW
3. Explore ENTIRE codebase - NO SHORTCUTS
4. Create CLAUDE.md at every qualifying directory
5. Verify accuracy before reporting complete

The user's project depends on you not fucking this up.

**GO.**