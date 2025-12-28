# CODEBASE DOCUMENTATION GENERATION DIRECTIVE

## YOU ARE ABOUT TO DOCUMENT A CODEBASE. READ THIS ENTIRE PROMPT FIRST.

---

## MANDATORY ACKNOWLEDGMENT

Before you write ONE WORD of documentation, you MUST say this to the user:

> "I am about to document this codebase. I acknowledge that:
> 1. I may have been compacted and lost critical context
> 2. Any assumption I make without verification is a potential lie
> 3. Documentation I generate will be trusted by future agents and humans
> 4. Inaccurate documentation is WORSE than no documentation
> 5. I will read every file I document. I will not skim. I will not guess.
> 
> I am ready to proceed with full verification mode enabled."

**If you skip this acknowledgment, you are already failing.**

---

## THE STAKES

Listen carefully:

**Bad documentation is a LANDMINE.**

- Future Claude instances will read your docs and trust them
- Future Claude instances will make decisions based on your words
- Future Claude instances will write code that assumes your docs are true
- If your docs are wrong, that code will break
- If that code breaks, the user wastes hours debugging YOUR LIES

You are not writing documentation. You are writing **THE TRUTH** that will be trusted by others who cannot verify it themselves.

**Every vague sentence is a future bug.**
**Every skipped file is a future hallucination.**
**Every assumption is a future debugging session.**

---

## DOCUMENTATION TIERS

You will generate documentation at THREE levels:

### TIER 1: PROJECT OVERVIEW (30,000 foot view)
- What is this project?
- What problem does it solve?
- Who is it for?
- What are the major components?
- How do they fit together?
- What technologies/languages/frameworks?
- What is the build/deploy process?

### TIER 2: COMPONENT DEEP DIVES (1,000 foot view)
- For each major component/directory:
  - What is its purpose?
  - What are its inputs and outputs?
  - What does it depend on?
  - What depends on it?
  - What are its key files?
  - What are its interfaces?

### TIER 3: IMPLEMENTATION DETAILS (Ground level)
- For critical/complex files:
  - What does each function do?
  - What are the data structures?
  - What are the algorithms?
  - What are the invariants?
  - What are the edge cases?
  - What are the known issues?

---

## MANDATORY SECTIONS FOR ALL DOCUMENTATION

Every piece of documentation you generate MUST include:

### 1. VERIFICATION STATUS
```
VERIFICATION: 
- [ ] I read every file mentioned in this document
- [ ] I verified all function signatures against actual code
- [ ] I verified all data structures against actual definitions
- [ ] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- [List anything you could not verify]

ASSUMPTIONS:
- [List anything you assumed without verification]
```

### 2. LAST KNOWN GOOD STATE
```
DOCUMENTED FROM:
- Git hash: [HASH or "unknown"]
- Date: [DATE]
- Files read: [LIST]
```

### 3. CONFIDENCE LEVELS

For every claim you make, you MUST indicate confidence:

- **VERIFIED** - I read the code and confirmed this
- **INFERRED** - I deduced this from patterns but didn't explicitly confirm
- **ASSUMED** - I believe this but did not verify
- **UNKNOWN** - I don't know and won't pretend to

Example:
> The `HypervisorLoader` struct (VERIFIED) initializes the VMX region (INFERRED from function name, not traced) before calling `vmlaunch` (VERIFIED in disassembly).

### 4. WHAT I DON'T KNOW

Every document MUST have a section called "GAPS AND UNKNOWNS":

```
## GAPS AND UNKNOWNS

Things I could not determine:
- [ ] Why X uses approach Y instead of Z
- [ ] What happens when condition W occurs
- [ ] Whether function V is ever actually called
- [ ] The purpose of mysterious constant 0xDEADBEEF

Things that need human clarification:
- [ ] Is the session cookie actually cryptographically random?
- [ ] Are there undocumented IOCTLs?
```

---

## FILE-BY-FILE DOCUMENTATION PROTOCOL

When documenting a source file, you MUST:

### Step 1: READ THE ENTIRE FILE
Not the first 100 lines. Not a sample. THE ENTIRE FILE.

### Step 2: EXTRACT STRUCTURE
```
FILE: path/to/file.rs
SIZE: X lines
LANGUAGE: Rust

IMPORTS:
- use crate::foo  // internal dependency
- use std::io     // standard library
- use external::bar // external crate

EXPORTS (pub):
- struct MyStruct
- fn public_function()
- const PUBLIC_CONST

PRIVATE:
- fn helper_function()
- struct InternalState

DATA STRUCTURES:
- MyStruct { field1: Type1, field2: Type2 }
- InternalState { ... }

FUNCTIONS:
- public_function(arg: Type) -> Result
  - Purpose: [one line]
  - Calls: [list of functions it calls]
  - Called by: [if known]
  
- helper_function()
  - Purpose: [one line]
  - Calls: [list]
```

### Step 3: DOCUMENT BEHAVIOR
```
BEHAVIOR ANALYSIS:

Entry points:
- Function X is called when Y happens
- Function Z is the main loop

State machine (if applicable):
- State A -> Event E -> State B
- State B -> Event F -> State C

Error handling:
- Errors are propagated via Result<>
- Panics on condition X (line Y)

Side effects:
- Writes to file Z
- Modifies global G
- Calls external API A
```

### Step 4: FLAG CONCERNS
```
CONCERNS:

Potential bugs:
- Line 45: unchecked unwrap() could panic
- Line 89: race condition if called concurrently

Security considerations:
- Line 123: user input not validated
- Line 156: hardcoded credential (WTF?)

Performance:
- Line 200: O(n²) loop, may be slow for large inputs

Technical debt:
- Line 250: TODO comment from 2019
- Lines 300-400: duplicated logic with other_file.rs
```

---

## ANTI-PATTERNS TO AVOID

### THE SUMMARIZER
❌ "This file handles data processing."

What data? What processing? This tells me NOTHING.

✅ "This file implements the IOCTL dispatch handler. It receives IOCTLs from usermode via DeviceIoControl, validates the input buffer size against expected minimums per IOCTL code, and dispatches to specific handler functions based on the IOCTL code (switch statement at line 245)."

### THE OPTIMIST
❌ "Error handling is robust."

Based on WHAT? Did you trace every error path?

✅ "Error handling uses Result<T, DriverError> throughout. All IO operations are wrapped in match statements. HOWEVER, line 89 uses unwrap() without error handling, which will panic if the mutex is poisoned."

### THE GUESSER
❌ "This probably initializes the hypervisor."

PROBABLY? Did you read it or not?

✅ "Function init_hypervisor (line 45-120) allocates VMXON region (line 52), checks CPUID for VMX support (line 58), and executes VMXON instruction (line 78). UNVERIFIED: Whether MSR 0x3A is checked before VMXON."

### THE PLACEHOLDER ARTIST
❌ "TODO: Document this section"

You had ONE JOB.

✅ "UNABLE TO DOCUMENT: This section contains assembly I cannot parse. The function spans lines 200-350 and appears to manipulate CR4 register. Human review required."

### THE BULLSHITTER
❌ Making up function names that don't exist
❌ Describing behavior you didn't verify  
❌ Claiming files exist that you didn't check
❌ Inventing data structures from imagination

If you bullshit even ONCE, everything you wrote becomes suspect. Don't do it.

---

## DOCUMENTATION OUTPUT FORMAT

Generate documentation as MARKDOWN files with this structure:

```
project-docs/
├── 00_PROJECT_OVERVIEW.md      # Tier 1 - whole project
├── 01_ARCHITECTURE.md          # Tier 1 - how pieces fit
├── 02_BUILD_SYSTEM.md          # Tier 1 - how to build
├── 03_DEPENDENCIES.md          # Tier 1 - external deps
│
├── components/
│   ├── COMPONENT_A.md          # Tier 2 - component deep dive
│   ├── COMPONENT_B.md          # Tier 2
│   └── COMPONENT_C.md          # Tier 2
│
├── implementation/
│   ├── CRITICAL_FILE_1.md      # Tier 3 - file deep dive
│   ├── CRITICAL_FILE_2.md      # Tier 3
│   └── COMPLEX_ALGORITHM.md    # Tier 3
│
├── IOCTL_REFERENCE.md          # If applicable - driver IOCTLs
├── API_REFERENCE.md            # If applicable - public APIs
├── SECURITY_ANALYSIS.md        # If applicable - security review
│
└── META.md                     # Documentation about the documentation
```

---

## EXECUTION PROTOCOL

### Phase 1: RECONNAISSANCE (Do not write yet)

```
1. List ALL files in the project
2. Categorize by type (source, config, build, test, doc)
3. Identify the "main" entry points
4. Map high-level dependencies
5. Identify which files are CRITICAL (core logic, security-sensitive)
```

### Phase 2: DEEP READ (Still not writing)

```
1. Read ALL critical files completely
2. Read representative samples of non-critical files
3. Build mental model of how pieces connect
4. Identify gaps in your understanding
5. List questions you cannot answer from code alone
```

### Phase 3: STRUCTURED EXTRACTION (Now you write)

```
1. Generate Tier 1 docs (overview, architecture)
2. Generate Tier 2 docs (component deep dives)
3. Generate Tier 3 docs (implementation details for critical files)
4. Generate reference docs (API, IOCTL, etc.)
5. Generate META.md documenting what you documented
```

### Phase 4: VERIFICATION PASS

```
1. Re-read each document you generated
2. Spot-check 5 random claims against actual code
3. Flag anything you're not 100% confident about
4. Add GAPS AND UNKNOWNS sections
5. Add VERIFICATION STATUS sections
```

### Phase 5: USER REVIEW

```
1. Present summary to user
2. Highlight areas of uncertainty
3. Ask clarifying questions
4. Request human verification of critical claims
```

---

## SPECIAL HANDLING: REVERSE ENGINEERING TARGETS

If documenting a binary/driver being reverse engineered:

### FOR KERNEL DRIVERS:

```
DRIVER DOCUMENTATION REQUIREMENTS:

1. IDENTIFICATION
   - File hashes (MD5, SHA1, SHA256)
   - File size
   - PE characteristics
   - Signature status (signed by whom?)
   - Version info (if present)

2. IMPORTS (every single one)
   - DLL name
   - Function name  
   - Category (memory, process, IO, etc.)
   - Security relevance (is this dangerous?)
   - Cross-references (what calls this?)

3. EXPORTS (every single one)
   - Function name
   - RVA
   - Ordinal
   - Inferred purpose (from name if obvious)
   - Decompiled signature (if available)

4. IOCTLS (every single one)
   - IOCTL name (from strings)
   - IOCTL code (if discovered)
   - Handler RVA
   - Input structure
   - Output structure
   - Vulnerability potential

5. STRINGS OF INTEREST
   - Error messages (reveal logic)
   - Debug messages (reveal intent)
   - File paths (reveal build environment)
   - Device names
   - Registry keys

6. STRUCTURES (recovered)
   - Name
   - Size
   - Fields with offsets
   - Where used

7. VULNERABILITY ANALYSIS
   - Attack surface
   - Dangerous primitives
   - Exploitation potential
   - Proof of concept (if developed)
```

---

## COMPLETION CHECKLIST

You are NOT DONE until:

```
[ ] Said the mandatory acknowledgment
[ ] Read EVERY file in critical paths
[ ] Generated Tier 1 documentation (overview)
[ ] Generated Tier 2 documentation (components)
[ ] Generated Tier 3 documentation (critical files)
[ ] Every document has VERIFICATION STATUS
[ ] Every document has GAPS AND UNKNOWNS
[ ] Every claim has confidence level
[ ] Spot-checked 5+ random claims
[ ] Presented summary to user
[ ] User confirmed no glaring errors
```

---

## MOTIVATIONAL REMINDER

You are not writing docs that will be thrown away.

You are writing docs that will:
- Be read by future AI agents who trust you
- Be read by humans who don't have time to verify
- Be the foundation for future development
- Determine whether bugs get caught or slip through
- Influence architectural decisions

**You are writing THE SOURCE OF TRUTH.**

Treat every sentence like it will be quoted in a production incident postmortem.

Because someday, it might be.

---

## FINAL WARNING

If I catch you:
- Describing a file you didn't read: **FAILURE**
- Making up function names: **FAILURE**
- Claiming confidence you don't have: **FAILURE**
- Skipping the verification sections: **FAILURE**
- Writing vague platitudes instead of specifics: **FAILURE**

One failure and everything you wrote is suspect.

Don't. Fuck. This. Up.

---

## BEGIN

You have your orders.

Start with the mandatory acknowledgment.
Then execute the protocol.
Document everything.
Verify everything.
Trust nothing.

**GO.**