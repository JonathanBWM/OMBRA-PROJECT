from pydantic import BaseModel, Field
from typing import Optional, List, Dict, Any
from datetime import datetime
from enum import Enum

class DriverStatus(str, Enum):
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    ANALYZED = "analyzed"
    BLOCKED = "blocked"

class VulnerabilityType(str, Enum):
    ARBITRARY_READ = "arbitrary_read"
    ARBITRARY_WRITE = "arbitrary_write"
    POOL_OVERFLOW = "pool_overflow"
    STACK_OVERFLOW = "stack_overflow"
    UAF = "use_after_free"
    DOUBLE_FETCH = "double_fetch"
    TOCTOU = "toctou"
    INTEGER_OVERFLOW = "integer_overflow"
    NULL_DEREF = "null_deref"
    IMPROPER_VALIDATION = "improper_validation"
    INFO_LEAK = "info_leak"
    OTHER = "other"

class VulnerabilitySeverity(str, Enum):
    CRITICAL = "critical"
    HIGH = "high"
    MEDIUM = "medium"
    LOW = "low"
    INFO = "info"

class ExploitDifficulty(str, Enum):
    TRIVIAL = "trivial"
    EASY = "easy"
    MODERATE = "moderate"
    HARD = "hard"
    EXPERT = "expert"

class XrefType(str, Enum):
    CALL = "call"
    DATA = "data"
    JUMP = "jump"
    READ = "read"
    WRITE = "write"

class Driver(BaseModel):
    id: Optional[int] = None
    name: str
    filename: str
    md5: str
    sha256: str
    file_size: int
    status: DriverStatus = DriverStatus.PENDING
    description: Optional[str] = None
    vendor: Optional[str] = None
    version: Optional[str] = None
    signing_cert: Optional[str] = None
    is_signed: bool = False
    is_vulnerable: bool = False
    ghidra_project: Optional[str] = None
    base_address: Optional[int] = None
    entry_point: Optional[int] = None
    created_at: Optional[datetime] = None
    updated_at: Optional[datetime] = None

class IOCTL(BaseModel):
    id: Optional[int] = None
    driver_id: int
    code: int
    name: Optional[str] = None
    description: Optional[str] = None
    handler_address: Optional[int] = None
    handler_name: Optional[str] = None
    input_buffer_type: Optional[str] = None
    output_buffer_type: Optional[str] = None
    input_size: Optional[int] = None
    output_size: Optional[int] = None
    requires_admin: bool = False
    validation_notes: Optional[str] = None
    is_vulnerable: bool = False
    vulnerability_description: Optional[str] = None

class Function(BaseModel):
    id: Optional[int] = None
    driver_id: int
    address: int
    name: str
    decompiled_code: Optional[str] = None
    disassembly: Optional[str] = None
    description: Optional[str] = None
    is_export: bool = False
    is_import: bool = False
    signature: Optional[str] = None
    call_convention: Optional[str] = None

class Structure(BaseModel):
    id: Optional[int] = None
    driver_id: int
    name: str
    size: int
    definition: str
    description: Optional[str] = None
    is_ioctl_buffer: bool = False

class Vulnerability(BaseModel):
    id: Optional[int] = None
    driver_id: int
    vuln_type: VulnerabilityType
    severity: VulnerabilitySeverity
    title: str
    description: str
    affected_function: Optional[str] = None
    affected_address: Optional[int] = None
    root_cause: Optional[str] = None
    exploitation_notes: Optional[str] = None
    exploit_difficulty: Optional[ExploitDifficulty] = None
    poc_code: Optional[str] = None
    cve_id: Optional[str] = None
    discovered_by: Optional[str] = None
    discovered_date: Optional[datetime] = None

class AttackChain(BaseModel):
    id: Optional[int] = None
    driver_id: int
    name: str
    description: str
    steps: List[Dict[str, Any]]
    vulnerability_ids: List[int]
    success_rate: Optional[float] = None
    requirements: Optional[str] = None

class AnalysisSession(BaseModel):
    id: Optional[int] = None
    driver_id: int
    analyst: str
    started_at: datetime
    ended_at: Optional[datetime] = None
    goals: Optional[str] = None
    findings_summary: Optional[str] = None

class AnalysisNote(BaseModel):
    id: Optional[int] = None
    session_id: int
    timestamp: datetime
    note_type: str
    content: str
    related_address: Optional[int] = None
    related_function: Optional[str] = None
    tags: Optional[List[str]] = None
