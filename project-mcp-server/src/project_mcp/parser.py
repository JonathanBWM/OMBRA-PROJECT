"""
C/C++ Parser Module

Uses regex-based parsing for function and struct extraction.
This is a practical approach that works for most C code without
requiring tree-sitter to be fully configured.

Patterns detected:
- Function definitions with signatures
- Struct/union/enum definitions
- TODO/FIXME comments
- Stub patterns (empty bodies, return 0, etc.)
"""

import re
import hashlib
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional


@dataclass
class FunctionInfo:
    """Information about a parsed function."""
    name: str
    signature: str
    return_type: str
    line_start: int
    line_end: int
    is_stub: bool = False
    stub_reason: Optional[str] = None
    doc_comment: Optional[str] = None
    body: str = ""


@dataclass
class StructInfo:
    """Information about a parsed struct/union/enum."""
    name: str
    kind: str  # struct, union, enum
    definition: str
    line_start: int
    line_end: int
    member_count: int = 0


@dataclass
class StubInfo:
    """Information about a detected stub."""
    line_number: int
    stub_type: str  # todo, fixme, not_implemented, return_zero, empty_body, placeholder
    stub_text: str
    context: str  # Lines around the stub
    function_name: Optional[str] = None


@dataclass
class FileAnalysis:
    """Complete analysis of a source file."""
    path: str
    language: str
    line_count: int
    content_hash: str
    functions: list[FunctionInfo] = field(default_factory=list)
    structs: list[StructInfo] = field(default_factory=list)
    stubs: list[StubInfo] = field(default_factory=list)
    todo_count: int = 0
    fixme_count: int = 0


# Regex patterns for C/C++ parsing
FUNCTION_PATTERN = re.compile(
    r'''
    # Optional storage class and modifiers
    (?:(?:static|inline|extern|__forceinline|FORCEINLINE|NTSTATUS|VOID|BOOLEAN|PVOID|ULONG|PULONG|SIZE_T)\s+)*
    # Return type (handles pointers, const, etc.)
    ([\w_]+(?:\s*\*+)?)\s+
    # Calling convention (optional)
    (?:__cdecl|__stdcall|__fastcall|NTAPI|WINAPI)?\s*
    # Function name
    ([\w_]+)\s*
    # Parameters
    \(([^)]*)\)\s*
    # Function body start
    \{
    ''',
    re.VERBOSE | re.MULTILINE
)

STRUCT_PATTERN = re.compile(
    r'''
    # struct/union/enum keyword
    (struct|union|enum)\s+
    # Optional tag name
    ([\w_]+)?\s*
    # Body
    \{([^}]*)\}
    ''',
    re.VERBOSE | re.MULTILINE | re.DOTALL
)

TYPEDEF_STRUCT_PATTERN = re.compile(
    r'''
    typedef\s+
    (struct|union|enum)\s*
    (?:[\w_]+)?\s*
    \{([^}]*)\}\s*
    ([\w_]+)\s*;
    ''',
    re.VERBOSE | re.MULTILINE | re.DOTALL
)

# Stub detection patterns
STUB_PATTERNS = {
    'todo': re.compile(r'//\s*TODO[:\s](.*)$|/\*\s*TODO[:\s](.*?)\*/', re.IGNORECASE | re.MULTILINE),
    'fixme': re.compile(r'//\s*FIXME[:\s](.*)$|/\*\s*FIXME[:\s](.*?)\*/', re.IGNORECASE | re.MULTILINE),
    'not_implemented': re.compile(r'(STATUS_NOT_IMPLEMENTED|NotImplemented|not\s+implemented)', re.IGNORECASE),
    'return_zero': re.compile(r'^\s*return\s+0\s*;\s*$', re.MULTILINE),
    'return_null': re.compile(r'^\s*return\s+(NULL|nullptr)\s*;\s*$', re.MULTILINE),
    'return_success': re.compile(r'^\s*return\s+STATUS_SUCCESS\s*;\s*$', re.MULTILINE),
    'dbgprint_stub': re.compile(r'DbgPrint\s*\(\s*"[^"]*not\s*impl', re.IGNORECASE),
    'placeholder': re.compile(r'//\s*(placeholder|stub|skeleton)', re.IGNORECASE),
    'unreferenced': re.compile(r'UNREFERENCED_PARAMETER\s*\('),
}


def get_file_language(path: str) -> str:
    """Determine the language based on file extension."""
    ext = Path(path).suffix.lower()
    language_map = {
        '.c': 'c',
        '.h': 'c_header',
        '.cpp': 'cpp',
        '.hpp': 'cpp_header',
        '.cxx': 'cpp',
        '.cc': 'cpp',
        '.asm': 'asm',
        '.inc': 'asm',
        '.py': 'python',
        '.rs': 'rust',
    }
    return language_map.get(ext, 'unknown')


def compute_content_hash(content: str) -> str:
    """Compute SHA256 hash of file content."""
    return hashlib.sha256(content.encode('utf-8', errors='replace')).hexdigest()


def find_matching_brace(content: str, start: int) -> int:
    """Find the position of the matching closing brace."""
    depth = 1
    pos = start
    in_string = False
    in_char = False
    in_line_comment = False
    in_block_comment = False

    while pos < len(content) and depth > 0:
        char = content[pos]
        prev_char = content[pos - 1] if pos > 0 else ''

        # Handle comments
        if not in_string and not in_char:
            if not in_block_comment and char == '/' and pos + 1 < len(content):
                next_char = content[pos + 1]
                if next_char == '/':
                    in_line_comment = True
                elif next_char == '*':
                    in_block_comment = True

            if in_line_comment and char == '\n':
                in_line_comment = False

            if in_block_comment and prev_char == '*' and char == '/':
                in_block_comment = False

        # Skip if in comment
        if in_line_comment or in_block_comment:
            pos += 1
            continue

        # Handle strings and chars
        if char == '"' and prev_char != '\\':
            in_string = not in_string
        elif char == "'" and prev_char != '\\':
            in_char = not in_char

        # Count braces
        if not in_string and not in_char:
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1

        pos += 1

    return pos


def extract_functions(content: str, lines: list[str]) -> list[FunctionInfo]:
    """Extract all function definitions from C/C++ code."""
    functions = []

    for match in FUNCTION_PATTERN.finditer(content):
        return_type = match.group(1).strip()
        func_name = match.group(2).strip()
        params = match.group(3).strip()

        # Skip if this looks like a macro or control structure
        if func_name.upper() in ('IF', 'WHILE', 'FOR', 'SWITCH', 'CATCH'):
            continue

        # Find the function body
        body_start = match.end() - 1  # Position of opening brace
        body_end = find_matching_brace(content, body_start + 1)
        body = content[body_start:body_end]

        # Calculate line numbers
        line_start = content[:match.start()].count('\n') + 1
        line_end = content[:body_end].count('\n') + 1

        # Build signature
        signature = f"{return_type} {func_name}({params})"

        # Check for doc comment (look at lines before function)
        doc_comment = None
        if line_start > 1:
            prev_lines = lines[max(0, line_start - 5):line_start - 1]
            for i, line in enumerate(reversed(prev_lines)):
                stripped = line.strip()
                if stripped.startswith('/*') or stripped.startswith('//') or stripped.startswith('*'):
                    # Found a comment
                    doc_lines = []
                    for j in range(len(prev_lines) - i - 1, len(prev_lines)):
                        doc_lines.append(prev_lines[j])
                    doc_comment = '\n'.join(doc_lines)
                    break
                elif stripped and not stripped.startswith('#'):
                    break  # Non-comment, non-empty line found

        # Check if this is a stub
        is_stub, stub_reason = detect_function_stub(body, func_name)

        functions.append(FunctionInfo(
            name=func_name,
            signature=signature,
            return_type=return_type,
            line_start=line_start,
            line_end=line_end,
            is_stub=is_stub,
            stub_reason=stub_reason,
            doc_comment=doc_comment,
            body=body,
        ))

    return functions


def detect_function_stub(body: str, func_name: str) -> tuple[bool, Optional[str]]:
    """
    Detect if a function body is a stub.

    Returns:
        (is_stub, stub_reason)
    """
    # Remove comments for analysis
    body_no_comments = re.sub(r'//.*$', '', body, flags=re.MULTILINE)
    body_no_comments = re.sub(r'/\*.*?\*/', '', body_no_comments, flags=re.DOTALL)

    # Get non-empty lines
    lines = [l.strip() for l in body_no_comments.split('\n') if l.strip()]

    # Filter out just braces
    code_lines = [l for l in lines if l not in ('{', '}')]

    # Empty body (only braces)
    if len(code_lines) == 0:
        return True, "empty_body"

    # Single return statement
    if len(code_lines) == 1:
        line = code_lines[0]
        if re.match(r'return\s+0\s*;', line):
            return True, "return_zero"
        if re.match(r'return\s+(NULL|nullptr)\s*;', line):
            return True, "return_null"
        if re.match(r'return\s+STATUS_SUCCESS\s*;', line):
            return True, "return_success"
        if re.match(r'return\s+STATUS_NOT_IMPLEMENTED\s*;', line):
            return True, "not_implemented"

    # Check for stub patterns in body
    body_lower = body.lower()

    if 'todo' in body_lower:
        return True, "todo"

    if 'fixme' in body_lower:
        return True, "fixme"

    if 'not implemented' in body_lower or 'notimplemented' in body_lower:
        return True, "not_implemented"

    if 'placeholder' in body_lower or 'stub' in body_lower:
        return True, "placeholder"

    # Check for UNREFERENCED_PARAMETER with minimal code
    if 'UNREFERENCED_PARAMETER' in body:
        # Count actual code lines (excluding UNREFERENCED_PARAMETER)
        actual_code = [l for l in code_lines if 'UNREFERENCED_PARAMETER' not in l]
        if len(actual_code) <= 1:
            return True, "placeholder"

    # DbgPrint("Not implemented")
    if STUB_PATTERNS['dbgprint_stub'].search(body):
        return True, "not_implemented"

    return False, None


def extract_structs(content: str) -> list[StructInfo]:
    """Extract all struct/union/enum definitions from C/C++ code."""
    structs = []

    # Handle typedef struct/union/enum
    for match in TYPEDEF_STRUCT_PATTERN.finditer(content):
        kind = match.group(1)
        body = match.group(2)
        name = match.group(3).strip()

        line_start = content[:match.start()].count('\n') + 1
        line_end = content[:match.end()].count('\n') + 1

        # Count members (rough estimate based on semicolons)
        member_count = body.count(';')

        structs.append(StructInfo(
            name=name,
            kind=kind,
            definition=match.group(0),
            line_start=line_start,
            line_end=line_end,
            member_count=member_count,
        ))

    # Handle regular struct/union/enum
    for match in STRUCT_PATTERN.finditer(content):
        kind = match.group(1)
        name = match.group(2)
        body = match.group(3)

        if name is None:
            continue  # Anonymous struct

        # Skip if this was already found as typedef
        if any(s.name == name for s in structs):
            continue

        line_start = content[:match.start()].count('\n') + 1
        line_end = content[:match.end()].count('\n') + 1

        member_count = body.count(';')

        structs.append(StructInfo(
            name=name,
            kind=kind,
            definition=match.group(0),
            line_start=line_start,
            line_end=line_end,
            member_count=member_count,
        ))

    return structs


def detect_stubs(content: str, lines: list[str], functions: list[FunctionInfo]) -> list[StubInfo]:
    """Detect all stub patterns in the file."""
    stubs = []

    # Check for TODO/FIXME comments
    for pattern_name, pattern in STUB_PATTERNS.items():
        if pattern_name in ('todo', 'fixme'):
            for match in pattern.finditer(content):
                line_num = content[:match.start()].count('\n') + 1
                stub_text = match.group(0)

                # Get context (2 lines before and after)
                start_line = max(0, line_num - 3)
                end_line = min(len(lines), line_num + 2)
                context = '\n'.join(lines[start_line:end_line])

                # Find if this is in a function
                func_name = None
                for func in functions:
                    if func.line_start <= line_num <= func.line_end:
                        func_name = func.name
                        break

                stubs.append(StubInfo(
                    line_number=line_num,
                    stub_type=pattern_name,
                    stub_text=stub_text,
                    context=context,
                    function_name=func_name,
                ))

    # Add function-level stubs (from function analysis)
    for func in functions:
        if func.is_stub and func.stub_reason:
            # Only add if we don't already have a stub for this function
            existing = any(s.function_name == func.name for s in stubs)
            if not existing:
                start_line = max(0, func.line_start - 1)
                end_line = min(len(lines), func.line_end)
                context = '\n'.join(lines[start_line:end_line])

                stubs.append(StubInfo(
                    line_number=func.line_start,
                    stub_type=func.stub_reason,
                    stub_text=func.body[:200],  # First 200 chars of body
                    context=context,
                    function_name=func.name,
                ))

    return stubs


def analyze_file(path: str) -> Optional[FileAnalysis]:
    """
    Perform complete analysis of a source file.

    Returns FileAnalysis with functions, structs, and stubs.
    Returns None if file cannot be read or is not a supported language.
    """
    path_obj = Path(path)

    if not path_obj.exists():
        return None

    language = get_file_language(path)
    if language == 'unknown':
        return None

    # Read file content
    try:
        content = path_obj.read_text(encoding='utf-8', errors='replace')
    except Exception:
        return None

    lines = content.split('\n')
    line_count = len(lines)
    content_hash = compute_content_hash(content)

    # Parse based on language
    if language in ('c', 'c_header', 'cpp', 'cpp_header'):
        functions = extract_functions(content, lines)
        structs = extract_structs(content)
        stubs = detect_stubs(content, lines, functions)
    else:
        # For other languages, just count TODOs/FIXMEs
        functions = []
        structs = []
        stubs = []
        for i, line in enumerate(lines):
            if 'TODO' in line.upper():
                stubs.append(StubInfo(
                    line_number=i + 1,
                    stub_type='todo',
                    stub_text=line.strip(),
                    context=line,
                ))
            elif 'FIXME' in line.upper():
                stubs.append(StubInfo(
                    line_number=i + 1,
                    stub_type='fixme',
                    stub_text=line.strip(),
                    context=line,
                ))

    # Count TODOs and FIXMEs
    todo_count = sum(1 for s in stubs if s.stub_type == 'todo')
    fixme_count = sum(1 for s in stubs if s.stub_type == 'fixme')

    return FileAnalysis(
        path=str(path_obj.absolute()),
        language=language,
        line_count=line_count,
        content_hash=content_hash,
        functions=functions,
        structs=structs,
        stubs=stubs,
        todo_count=todo_count,
        fixme_count=fixme_count,
    )


def scan_directory(
    directory: str,
    extensions: Optional[list[str]] = None,
    recursive: bool = True,
) -> list[FileAnalysis]:
    """
    Scan a directory for source files and analyze them.

    Args:
        directory: Path to directory
        extensions: List of extensions to include (e.g., ['.c', '.h'])
        recursive: Whether to scan subdirectories

    Returns:
        List of FileAnalysis for all analyzed files
    """
    if extensions is None:
        extensions = ['.c', '.h', '.cpp', '.hpp', '.asm', '.py']

    results = []
    dir_path = Path(directory)

    if not dir_path.exists():
        return results

    # Get all files
    if recursive:
        files = dir_path.rglob('*')
    else:
        files = dir_path.glob('*')

    for file_path in files:
        if file_path.is_file() and file_path.suffix.lower() in extensions:
            analysis = analyze_file(str(file_path))
            if analysis:
                results.append(analysis)

    return results
