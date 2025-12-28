"""Pydantic models for Driver RE MCP database entities."""

from datetime import datetime
from typing import Optional, List, Dict, Any
from pydantic import BaseModel, Field
from uuid import uuid4


def generate_uuid() -> str:
    """Generate a UUID string."""
    return str(uuid4())


# ============================================
# DRIVER MODELS
# ============================================

class Driver(BaseModel):
    """Driver metadata."""
    id: str = Field(default_factory=generate_uuid)
    original_name: str
    analyzed_name: Optional[str] = None
    md5: str
    sha1: str
    sha256: str
    imphash: Optional[str] = None
    file_size: int
    image_base: int
    entry_point_rva: int
    size_of_image: int
    timestamp: Optional[int] = None
    machine: int
    subsystem: int
    characteristics: int
    file_version: Optional[str] = None
    product_version: Optional[str] = None
    company_name: Optional[str] = None
    product_name: Optional[str] = None
    file_description: Optional[str] = None
    build_path: Optional[str] = None
    pdb_path: Optional[str] = None
    analysis_status: str = "pending"
    notes: Optional[str] = None
    tags: Optional[str] = None  # JSON array
    created_at: Optional[str] = None
    updated_at: Optional[str] = None


class Section(BaseModel):
    """PE Section."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: str
    virtual_address: int
    virtual_size: int
    raw_address: int
    raw_size: int
    characteristics: int
    entropy: Optional[float] = None
    created_at: Optional[str] = None


# ============================================
# IMPORT/EXPORT MODELS
# ============================================

class Import(BaseModel):
    """Imported function."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    dll_name: str
    function_name: Optional[str] = None
    ordinal: Optional[int] = None
    hint: Optional[int] = None
    iat_rva: Optional[int] = None
    category: Optional[str] = None
    subcategory: Optional[str] = None
    is_dangerous: bool = False
    danger_reason: Optional[str] = None
    description: Optional[str] = None
    usage_count: int = 0
    created_at: Optional[str] = None


class Export(BaseModel):
    """Exported function."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    function_name: Optional[str] = None
    ordinal: int
    rva: int
    prefix: Optional[str] = None
    category: Optional[str] = None
    is_dangerous: bool = False
    danger_reason: Optional[str] = None
    return_type: Optional[str] = None
    parameters: Optional[str] = None  # JSON
    calling_convention: Optional[str] = None
    description: Optional[str] = None
    decompiled_code: Optional[str] = None
    created_at: Optional[str] = None


# ============================================
# IOCTL MODEL
# ============================================

class IOCTL(BaseModel):
    """IOCTL handler."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: str
    code: Optional[int] = None
    code_hex: Optional[str] = None
    device_type: Optional[int] = None
    function_code: Optional[int] = None
    method: Optional[int] = None
    access: Optional[int] = None
    handler_rva: Optional[int] = None
    handler_function_id: Optional[str] = None
    input_struct_id: Optional[str] = None
    output_struct_id: Optional[str] = None
    min_input_size: Optional[int] = None
    max_input_size: Optional[int] = None
    min_output_size: Optional[int] = None
    max_output_size: Optional[int] = None
    requires_admin: Optional[bool] = None
    requires_session: bool = True
    is_vulnerable: bool = False
    vulnerability_type: Optional[str] = None
    vulnerability_severity: Optional[str] = None
    vulnerability_description: Optional[str] = None
    exploitation_notes: Optional[str] = None
    cve_ids: Optional[str] = None  # JSON array
    description: Optional[str] = None
    has_fast_path: bool = False
    created_at: Optional[str] = None


# ============================================
# STRUCTURE MODELS
# ============================================

class StructureMember(BaseModel):
    """Structure member/field."""
    id: str = Field(default_factory=generate_uuid)
    structure_id: str
    name: str
    offset: int
    size: int
    type_name: Optional[str] = None
    nested_struct_id: Optional[str] = None
    is_array: bool = False
    array_count: Optional[int] = None
    is_pointer: bool = False
    pointer_depth: int = 0
    is_bitfield: bool = False
    bit_offset: Optional[int] = None
    bit_size: Optional[int] = None
    description: Optional[str] = None
    created_at: Optional[str] = None


class Structure(BaseModel):
    """Data structure definition."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: Optional[str] = None
    name: str
    size: Optional[int] = None
    alignment: Optional[int] = None
    struct_type: Optional[str] = None
    definition_c: Optional[str] = None
    definition_json: Optional[str] = None  # JSON
    source: Optional[str] = None
    description: Optional[str] = None
    created_at: Optional[str] = None
    members: List[StructureMember] = []


# ============================================
# FUNCTION MODEL
# ============================================

class Function(BaseModel):
    """Analyzed function."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: Optional[str] = None
    rva: int
    size: Optional[int] = None
    source: Optional[str] = None
    return_type: Optional[str] = None
    parameters: Optional[str] = None  # JSON
    calling_convention: str = "fastcall"
    is_entry_point: bool = False
    is_dispatch: bool = False
    dispatch_type: Optional[str] = None
    assembly: Optional[str] = None
    decompiled: Optional[str] = None
    pseudocode: Optional[str] = None
    annotations: Optional[str] = None  # JSON
    cyclomatic_complexity: Optional[int] = None
    basic_block_count: Optional[int] = None
    instruction_count: Optional[int] = None
    ghidra_synced: bool = False
    ghidra_sync_time: Optional[str] = None
    created_at: Optional[str] = None
    updated_at: Optional[str] = None


# ============================================
# CROSS-REFERENCE MODEL
# ============================================

class XRef(BaseModel):
    """Cross-reference."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    from_rva: int
    from_function_id: Optional[str] = None
    to_rva: int
    to_function_id: Optional[str] = None
    to_import_id: Optional[str] = None
    to_export_id: Optional[str] = None
    xref_type: str  # call, jump, data_ref, offset
    instruction: Optional[str] = None
    created_at: Optional[str] = None


# ============================================
# GLOBAL VARIABLE MODEL
# ============================================

class Global(BaseModel):
    """Global variable."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: Optional[str] = None
    rva: int
    size: Optional[int] = None
    type_name: Optional[str] = None
    structure_id: Optional[str] = None
    initial_value_hex: Optional[str] = None
    section_name: Optional[str] = None
    is_sensitive: bool = False
    sensitivity_reason: Optional[str] = None
    description: Optional[str] = None
    created_at: Optional[str] = None


# ============================================
# STRING MODEL
# ============================================

class String(BaseModel):
    """String constant."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    value: str
    rva: int
    length: int
    encoding: Optional[str] = None
    section_name: Optional[str] = None
    category: Optional[str] = None
    reference_count: int = 0
    created_at: Optional[str] = None


# ============================================
# VULNERABILITY MODELS
# ============================================

class Vulnerability(BaseModel):
    """Vulnerability finding."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    title: str
    vulnerability_class: str
    severity: str
    cvss_score: Optional[float] = None
    cvss_vector: Optional[str] = None
    cve_id: Optional[str] = None
    cve_url: Optional[str] = None
    affected_ioctl_id: Optional[str] = None
    affected_function_id: Optional[str] = None
    affected_export_id: Optional[str] = None
    description: str
    technical_details: Optional[str] = None
    exploitation_difficulty: Optional[str] = None
    exploitation_requirements: Optional[str] = None
    exploitation_steps: Optional[str] = None  # JSON
    poc_code: Optional[str] = None
    poc_language: Optional[str] = None
    mitigations: Optional[str] = None
    references: Optional[str] = None  # JSON
    status: str = "confirmed"
    created_at: Optional[str] = None
    updated_at: Optional[str] = None


class AttackChain(BaseModel):
    """Attack chain combining multiple vulnerabilities."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: str
    description: Optional[str] = None
    attack_goal: Optional[str] = None
    steps: str  # JSON
    initial_access: Optional[str] = None
    final_privilege: Optional[str] = None
    poc_code: Optional[str] = None
    created_at: Optional[str] = None


# ============================================
# API CATEGORY MODEL
# ============================================

class APICategory(BaseModel):
    """API category reference."""
    id: str = Field(default_factory=generate_uuid)
    category: str
    subcategory: Optional[str] = None
    description: Optional[str] = None
    security_relevance: Optional[str] = None
    common_misuse: Optional[str] = None
    security_notes: Optional[str] = None
    created_at: Optional[str] = None


# ============================================
# ANALYSIS SESSION MODELS
# ============================================

class AnalysisSession(BaseModel):
    """Analysis session."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    name: Optional[str] = None
    ghidra_project_path: Optional[str] = None
    ghidra_project_name: Optional[str] = None
    status: str = "active"
    notes: Optional[str] = None
    started_at: Optional[str] = None
    ended_at: Optional[str] = None
    created_at: Optional[str] = None


class AnalysisNote(BaseModel):
    """Analysis note."""
    id: str = Field(default_factory=generate_uuid)
    driver_id: str
    session_id: Optional[str] = None
    related_function_id: Optional[str] = None
    related_ioctl_id: Optional[str] = None
    related_vuln_id: Optional[str] = None
    rva: Optional[int] = None
    title: Optional[str] = None
    content: str
    note_type: Optional[str] = None
    priority: Optional[str] = None
    created_at: Optional[str] = None
    updated_at: Optional[str] = None
