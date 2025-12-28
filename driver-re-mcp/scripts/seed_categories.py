#!/usr/bin/env python3
"""Seed API categories reference data"""

import asyncio
import sys
import uuid
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from driver_re_mcp.config import settings
from driver_re_mcp.database import init_database, get_db

API_CATEGORIES = [
    {
        'id': str(uuid.uuid4()),
        'category': 'Memory Management - Pool',
        'subcategory': 'Pool Allocation',
        'description': 'Kernel pool memory allocation and manipulation',
        'security_relevance': 'Pool corruptions, UAF, arbitrary writes',
        'common_misuse': 'Insufficient size validation, missing bounds checks',
        'security_notes': 'ExAllocatePoolWithTag requires careful size validation'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Memory Management - Physical',
        'subcategory': 'Physical Memory',
        'description': 'Direct physical memory access',
        'security_relevance': 'Arbitrary read/write primitives, hypervisor attacks',
        'common_misuse': 'MmMapIoSpace without access checks, physical memory R/W',
        'security_notes': 'Physical memory access = kernel code execution primitive'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Process/Thread',
        'subcategory': 'Process Manipulation',
        'description': 'Process and thread creation/modification',
        'security_relevance': 'Privilege escalation, code injection',
        'common_misuse': 'Missing PreviousMode checks, insufficient validation',
        'security_notes': 'KeStackAttachProcess requires careful privilege validation'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Registry',
        'subcategory': 'Registry Access',
        'description': 'Windows registry read/write operations',
        'security_relevance': 'Persistence, configuration tampering',
        'common_misuse': 'Writing to protected keys without checks',
        'security_notes': 'Validate key paths and access rights'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'File I/O',
        'subcategory': 'File System',
        'description': 'File system read/write operations',
        'security_relevance': 'Arbitrary file write, bootkit installation',
        'common_misuse': 'Path traversal, insufficient ACL checks',
        'security_notes': 'Validate file paths, check FILE_DIRECTORY_FILE flag'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Device I/O',
        'subcategory': 'Driver Communication',
        'description': 'Inter-driver communication via IOCTLs',
        'security_relevance': 'Chained exploits, driver proxy attacks',
        'common_misuse': 'Forwarding user buffers without re-validation',
        'security_notes': 'Re-validate all parameters when forwarding IOCTLs'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Hardware',
        'subcategory': 'Port I/O',
        'description': 'Direct hardware port access',
        'security_relevance': 'SMM code execution, firmware attacks',
        'common_misuse': 'Exposing port I/O to usermode',
        'security_notes': 'READ_PORT_*/WRITE_PORT_* = hardware control'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Security',
        'subcategory': 'Access Control',
        'description': 'Security descriptors and token manipulation',
        'security_relevance': 'Privilege escalation, ACL bypass',
        'common_misuse': 'Missing SeAccessCheck calls',
        'security_notes': 'Always validate security context before privileged operations'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Virtualization',
        'subcategory': 'Hypervisor',
        'description': 'VMX/SVM virtualization operations',
        'security_relevance': 'Ring -1 code execution, VM escape',
        'common_misuse': 'Exposing VMX instructions to usermode',
        'security_notes': '__vmx_* intrinsics = hypervisor installation primitive'
    },
    {
        'id': str(uuid.uuid4()),
        'category': 'Object Management',
        'subcategory': 'Object References',
        'description': 'Kernel object reference counting',
        'security_relevance': 'UAF vulnerabilities',
        'common_misuse': 'Mismatched Ob{Reference,Dereference}Object calls',
        'security_notes': 'Reference counting errors = UAF conditions'
    },
]

async def main():
    """Seed API categories"""
    print(f"Seeding API categories into: {settings.DATABASE_PATH}")

    db_manager = init_database(settings.database_path_obj)

    try:
        await db_manager.connect()

        # Check if already seeded
        count_result = await db_manager.fetch_one("SELECT COUNT(*) as count FROM api_categories")
        if count_result and count_result['count'] > 0:
            print(f"API categories already seeded ({count_result['count']} categories)")
            return

        # Insert categories
        for category in API_CATEGORIES:
            await db_manager.insert('api_categories', category)

        print(f"Successfully seeded {len(API_CATEGORIES)} API categories")

    except Exception as e:
        print(f"Error seeding categories: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        await db_manager.close()

if __name__ == "__main__":
    asyncio.run(main())
