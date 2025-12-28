"""IOCTL analysis tools"""

import uuid
from typing import Optional
from ..database import get_db

async def add_ioctl(
    driver_id: str,
    name: str,
    code: int,
    handler_rva: Optional[int] = None,
    handler_function_id: Optional[str] = None,
    input_struct_id: Optional[str] = None,
    output_struct_id: Optional[str] = None,
    min_input_size: Optional[int] = None,
    max_input_size: Optional[int] = None,
    min_output_size: Optional[int] = None,
    max_output_size: Optional[int] = None,
    requires_admin: Optional[bool] = None,
    is_vulnerable: Optional[bool] = None,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None,
    description: Optional[str] = None,
    **kwargs
) -> dict:
    """Add IOCTL to driver with full CTL_CODE decoding and vulnerability tracking"""
    db = get_db()

    # Verify driver exists
    driver = await db.fetch_one("SELECT id FROM drivers WHERE id = ?", (driver_id,))
    if not driver:
        return {'success': False, 'error': 'Driver not found'}

    ioctl_id = str(uuid.uuid4())
    code_hex = f"0x{code:08X}"

    # Decode CTL_CODE components: CTL_CODE(DeviceType, Function, Method, Access)
    device_type = (code >> 16) & 0xFFFF
    access = (code >> 14) & 0x3
    function_code = (code >> 2) & 0xFFF
    method = code & 0x3

    # Method names for reference
    method_names = {0: 'METHOD_BUFFERED', 1: 'METHOD_IN_DIRECT', 2: 'METHOD_OUT_DIRECT', 3: 'METHOD_NEITHER'}
    access_names = {0: 'FILE_ANY_ACCESS', 1: 'FILE_READ_ACCESS', 2: 'FILE_WRITE_ACCESS', 3: 'FILE_READ_WRITE_ACCESS'}

    data = {
        'id': ioctl_id,
        'driver_id': driver_id,
        'name': name,
        'code': code,
        'code_hex': code_hex,
        'device_type': device_type,
        'function_code': function_code,
        'method': method,
        'access': access,
        'handler_rva': handler_rva,
        'handler_function_id': handler_function_id,
        'input_struct_id': input_struct_id,
        'output_struct_id': output_struct_id,
        'min_input_size': min_input_size,
        'max_input_size': max_input_size,
        'min_output_size': min_output_size,
        'max_output_size': max_output_size,
        'requires_admin': 1 if requires_admin else 0 if requires_admin is False else None,
        'is_vulnerable': 1 if is_vulnerable else 0,
        'vulnerability_type': vulnerability_type,
        'vulnerability_severity': vulnerability_severity,
        'vulnerability_description': vulnerability_description,
        'description': description or ''
    }

    # Remove None values to let DB use defaults
    data = {k: v for k, v in data.items() if v is not None}

    await db.insert('ioctls', data)

    return {
        'success': True,
        'ioctl_id': ioctl_id,
        'ctl_code_decoded': {
            'device_type': f"0x{device_type:04X}",
            'function_code': f"0x{function_code:03X}",
            'method': method_names.get(method, f"UNKNOWN({method})"),
            'access': access_names.get(access, f"UNKNOWN({access})")
        },
        'message': f'IOCTL {name} ({code_hex}) added'
    }

async def get_ioctl(
    ioctl_id: Optional[str] = None,
    driver_id: Optional[str] = None,
    code: Optional[int] = None
) -> dict:
    """Get IOCTL by ID or code"""
    db = get_db()

    if ioctl_id:
        ioctl = await db.fetch_one("SELECT * FROM ioctls WHERE id = ?", (ioctl_id,))
    elif driver_id and code is not None:
        ioctl = await db.fetch_one(
            "SELECT * FROM ioctls WHERE driver_id = ? AND code = ?",
            (driver_id, code)
        )
    else:
        return {'success': False, 'error': 'Must provide ioctl_id or (driver_id + code)'}

    if not ioctl:
        return {'success': False, 'error': 'IOCTL not found'}

    return {
        'success': True,
        'ioctl': ioctl
    }

async def list_ioctls(driver_id: str) -> dict:
    """List all IOCTLs for driver"""
    db = get_db()

    ioctls = await db.fetch_all(
        "SELECT * FROM ioctls WHERE driver_id = ? ORDER BY code",
        (driver_id,)
    )

    return {
        'success': True,
        'count': len(ioctls),
        'ioctls': ioctls
    }

async def get_vulnerable_ioctls(
    driver_id: Optional[str] = None,
    severity: Optional[str] = None
) -> dict:
    """Get vulnerable IOCTLs"""
    db = get_db()

    query = "SELECT * FROM ioctls WHERE is_vulnerable = 1"
    params = []

    if driver_id:
        query += " AND driver_id = ?"
        params.append(driver_id)

    if severity:
        query += " AND vulnerability_severity = ?"
        params.append(severity)

    query += " ORDER BY vulnerability_severity DESC"

    ioctls = await db.fetch_all(query, tuple(params))

    return {
        'success': True,
        'count': len(ioctls),
        'ioctls': ioctls
    }

async def update_ioctl_vulnerability(
    ioctl_id: str,
    is_vulnerable: Optional[bool] = None,
    vulnerability_type: Optional[str] = None,
    vulnerability_severity: Optional[str] = None,
    vulnerability_description: Optional[str] = None
) -> dict:
    """Update IOCTL vulnerability information"""
    db = get_db()

    # Verify IOCTL exists
    ioctl = await db.fetch_one("SELECT id FROM ioctls WHERE id = ?", (ioctl_id,))
    if not ioctl:
        return {'success': False, 'error': 'IOCTL not found'}

    update_data = {}
    if is_vulnerable is not None:
        update_data['is_vulnerable'] = 1 if is_vulnerable else 0
    if vulnerability_type:
        update_data['vulnerability_type'] = vulnerability_type
    if vulnerability_severity:
        update_data['vulnerability_severity'] = vulnerability_severity
    if vulnerability_description:
        update_data['vulnerability_description'] = vulnerability_description

    if not update_data:
        return {'success': False, 'error': 'No update data provided'}

    await db.update('ioctls', ioctl_id, update_data)

    return {
        'success': True,
        'message': 'IOCTL vulnerability info updated'
    }
