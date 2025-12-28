# September 2025 - Week 5
# Channel: #programming
# Messages: 2

[2025-09-29 09:35] Xits: Turns out if no code references a BB llvm just removes the BB even if a global references it

[2025-09-29 09:35] Xits: Because I wasn’t explicitly specifying targets of an indirect branch(which seems obvious in hindsight)