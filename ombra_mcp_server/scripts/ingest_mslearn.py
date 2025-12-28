#!/usr/bin/env python3
"""
Ingest MS Learn pages into the reference database with intelligent chunking.

Parses pages into semantic chunks (~200-400 words), extracts API references,
code examples, and builds the searchable knowledge base.

Usage:
    # Ingest a single URL
    python ingest_mslearn.py "https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-mdls"

    # Ingest from TOC with topic filter
    python ingest_mslearn.py --toc "https://learn.microsoft.com/.../kernel/" --topics MDL "Physical Memory"

    # Ingest specific URLs from file
    python ingest_mslearn.py --file urls.txt
"""

import sqlite3
import requests
from bs4 import BeautifulSoup
import re
import json
import argparse
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Optional, Tuple
from urllib.parse import urljoin, urlparse

DATA_DIR = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data"
DB_PATH = DATA_DIR / "mslearn_reference.db"

HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36',
    'Accept': 'text/html,application/xhtml+xml',
}

# Known Windows kernel APIs - for extraction and linking
KNOWN_APIS = {
    # MDL functions
    'IoAllocateMdl', 'IoFreeMdl', 'IoBuildPartialMdl',
    'MmInitializeMdl', 'MmSizeOfMdl', 'MmPrepareMdlForReuse',
    'MmBuildMdlForNonPagedPool', 'MmProbeAndLockPages', 'MmUnlockPages',
    'MmMapLockedPages', 'MmMapLockedPagesSpecifyCache', 'MmUnmapLockedPages',
    'MmGetSystemAddressForMdl', 'MmGetSystemAddressForMdlSafe',
    'MmGetMdlVirtualAddress', 'MmGetMdlByteCount', 'MmGetMdlByteOffset',
    'MmGetMdlPfnArray',
    # Memory allocation
    'ExAllocatePool', 'ExAllocatePoolWithTag', 'ExAllocatePool2',
    'ExFreePool', 'ExFreePoolWithTag',
    'MmAllocateContiguousMemory', 'MmFreeContiguousMemory',
    'MmAllocatePagesForMdl', 'MmAllocatePagesForMdlEx', 'MmFreePagesFromMdl',
    'MmAllocateNonCachedMemory', 'MmFreeNonCachedMemory',
    'MmAllocateMappingAddress', 'MmFreeMappingAddress',
    # Physical memory
    'MmMapIoSpace', 'MmUnmapIoSpace',
    'MmGetPhysicalAddress', 'MmGetVirtualForPhysical',
    'MmCopyMemory',
    # Virtual memory
    'MmIsAddressValid', 'MmIsNonPagedSystemAddressValid',
    'ZwAllocateVirtualMemory', 'ZwFreeVirtualMemory',
    'ZwProtectVirtualMemory', 'ZwQueryVirtualMemory',
    # Driver/device
    'IoCreateDevice', 'IoDeleteDevice', 'IoAttachDevice', 'IoDetachDevice',
    'IoCreateDriver', 'IoDeleteDriver',
    'IoCreateSymbolicLink', 'IoDeleteSymbolicLink',
    'IoGetDeviceObjectPointer', 'ObReferenceObjectByHandle',
    # IRP handling
    'IoCallDriver', 'IoCompleteRequest', 'IoCopyCurrentIrpStackLocationToNext',
    'IoGetCurrentIrpStackLocation', 'IoGetNextIrpStackLocation',
    'IoMarkIrpPending', 'IoStartPacket', 'IoStartNextPacket',
    # Synchronization
    'KeInitializeSpinLock', 'KeAcquireSpinLock', 'KeReleaseSpinLock',
    'KeAcquireSpinLockAtDpcLevel', 'KeReleaseSpinLockFromDpcLevel',
    'ExAcquireFastMutex', 'ExReleaseFastMutex',
    'KeWaitForSingleObject', 'KeWaitForMultipleObjects',
    # DPC/interrupts
    'KeInitializeDpc', 'KeInsertQueueDpc', 'KeRemoveQueueDpc',
    'IoConnectInterrupt', 'IoDisconnectInterrupt',
    'KeSynchronizeExecution',
    # Process/thread
    'PsGetCurrentProcess', 'PsGetCurrentThread', 'PsGetCurrentProcessId',
    'KeStackAttachProcess', 'KeUnstackDetachProcess',
    'ZwOpenProcess', 'ZwQueryInformationProcess',
    # Registry
    'ZwOpenKey', 'ZwCreateKey', 'ZwQueryValueKey', 'ZwSetValueKey',
    # File I/O
    'ZwCreateFile', 'ZwReadFile', 'ZwWriteFile', 'ZwClose',
}

# Structures we care about
KNOWN_STRUCTURES = {
    'MDL', '_MDL', 'PMDL',
    'IRP', '_IRP', 'PIRP',
    'DRIVER_OBJECT', '_DRIVER_OBJECT', 'PDRIVER_OBJECT',
    'DEVICE_OBJECT', '_DEVICE_OBJECT', 'PDEVICE_OBJECT',
    'IO_STACK_LOCATION', 'PIO_STACK_LOCATION',
    'PHYSICAL_ADDRESS', 'LARGE_INTEGER',
    'EPROCESS', '_EPROCESS', 'PEPROCESS',
    'ETHREAD', '_ETHREAD', 'PETHREAD',
    'KTHREAD', '_KTHREAD', 'PKTHREAD',
    'PFN_NUMBER', 'MMPFN', '_MMPFN',
}


def get_conn():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def fetch_page(url: str) -> Optional[Dict]:
    """Fetch and parse a MS Learn page."""
    try:
        response = requests.get(url, headers=HEADERS, timeout=30)
        response.raise_for_status()
    except requests.RequestException as e:
        print(f"  ERROR fetching {url}: {e}")
        return None

    soup = BeautifulSoup(response.text, 'html.parser')

    # Title
    title = None
    title_elem = soup.select_one('h1')
    if title_elem:
        title = title_elem.get_text(strip=True)

    # Breadcrumb
    breadcrumb = []
    for crumb in soup.select('nav[aria-label="Breadcrumb"] a, .breadcrumbs a'):
        breadcrumb.append(crumb.get_text(strip=True))
    toc_path = ' > '.join(breadcrumb) if breadcrumb else None

    # Last updated
    updated = None
    time_elem = soup.select_one('time')
    if time_elem:
        updated = time_elem.get('datetime') or time_elem.get_text(strip=True)

    # Main content
    main_content = soup.select_one('main, article, .content, #main-column')
    if not main_content:
        return None

    return {
        'url': url,
        'title': title,
        'toc_path': toc_path,
        'updated': updated,
        'soup': main_content,
    }


def extract_apis_from_text(text: str) -> List[str]:
    """Extract known API names from text."""
    found = []
    for api in KNOWN_APIS | KNOWN_STRUCTURES:
        # Match API name - MS Learn sometimes strips spaces so also check without word boundaries
        if api in text:
            found.append(api)
    return list(set(found))


def classify_chunk(heading: str, content: str) -> str:
    """Classify a chunk by its type."""
    heading_lower = (heading or '').lower()
    content_lower = content.lower()

    if 'example' in heading_lower or 'sample' in heading_lower:
        return 'example'
    if 'warning' in heading_lower or 'caution' in heading_lower or 'important' in heading_lower:
        return 'warning'
    if 'note' in heading_lower:
        return 'note'
    if 'parameter' in heading_lower or 'return' in heading_lower:
        return 'api_detail'
    if 'see also' in heading_lower or 'related' in heading_lower:
        return 'related'
    if any(api in content for api in KNOWN_APIS):
        if content_lower.count('routine') > 2 or content_lower.count('function') > 2:
            return 'api_list'
    if 'overview' in heading_lower or heading_lower == '' or 'introduction' in heading_lower:
        return 'overview'

    return 'usage'


def chunk_page(soup: BeautifulSoup) -> List[Dict]:
    """
    Chunk a page into semantic units.

    Strategy:
    - Each h2/h3 section becomes a potential chunk
    - Very long sections get split at paragraph boundaries
    - Code blocks are kept as separate units
    - Target chunk size: 200-400 words
    """
    chunks = []
    current_chunk = {
        'heading': None,
        'content': '',
        'code_blocks': [],
    }

    def finalize_chunk():
        if current_chunk['content'].strip():
            words = len(current_chunk['content'].split())
            chunks.append({
                'heading': current_chunk['heading'],
                'content': current_chunk['content'].strip(),
                'word_count': words,
                'code_blocks': current_chunk['code_blocks'].copy(),
                'concept_type': classify_chunk(current_chunk['heading'], current_chunk['content']),
                'apis': extract_apis_from_text(current_chunk['content']),
            })
        current_chunk['content'] = ''
        current_chunk['code_blocks'] = []

    for elem in soup.find_all(['h1', 'h2', 'h3', 'h4', 'p', 'ul', 'ol', 'table', 'pre', 'blockquote', 'div']):
        # Skip navigation, feedback sections
        if elem.get('class'):
            classes = ' '.join(elem.get('class', []))
            if any(skip in classes for skip in ['feedback', 'navigation', 'sidebar', 'footer']):
                continue

        if elem.name in ['h1', 'h2', 'h3']:
            # New section - finalize previous chunk
            finalize_chunk()
            current_chunk['heading'] = elem.get_text(strip=True)

        elif elem.name == 'h4':
            # Sub-section - might start new chunk if current is long
            words = len(current_chunk['content'].split())
            if words > 300:
                finalize_chunk()
                current_chunk['heading'] = elem.get_text(strip=True)
            else:
                current_chunk['content'] += f"\n\n**{elem.get_text(strip=True)}**\n"

        elif elem.name == 'pre':
            # Code block - extract and store separately
            code = elem.get_text(strip=False)
            if len(code.strip()) > 10:
                current_chunk['code_blocks'].append(code.strip())
                current_chunk['content'] += f"\n[CODE BLOCK: {len(code.split(chr(10)))} lines]\n"

        elif elem.name == 'table':
            # Extract table as text
            rows = []
            for tr in elem.select('tr'):
                cells = [td.get_text(strip=True) for td in tr.select('th, td')]
                if cells:
                    rows.append(' | '.join(cells))
            if rows:
                current_chunk['content'] += '\n' + '\n'.join(rows) + '\n'

        elif elem.name in ['ul', 'ol']:
            # List items
            items = []
            for li in elem.select('li'):
                items.append('• ' + li.get_text(strip=True))
            if items:
                current_chunk['content'] += '\n' + '\n'.join(items) + '\n'

        elif elem.name in ['p', 'blockquote']:
            text = elem.get_text(strip=True)
            if text:
                current_chunk['content'] += '\n' + text + '\n'

                # Check if chunk is getting too long
                words = len(current_chunk['content'].split())
                if words > 500:
                    finalize_chunk()
                    # Continue with same heading for next chunk
                    # (it's a continuation)

        elif elem.name == 'div':
            # Some content is in divs
            if not elem.find(['h1', 'h2', 'h3', 'h4', 'pre', 'table']):
                text = elem.get_text(strip=True)
                if text and len(text) > 20:
                    current_chunk['content'] += '\n' + text + '\n'

    # Don't forget the last chunk
    finalize_chunk()

    return chunks


def extract_code_examples(chunks: List[Dict]) -> List[Dict]:
    """Extract code examples from chunks."""
    examples = []
    for i, chunk in enumerate(chunks):
        for code in chunk.get('code_blocks', []):
            # Detect language
            language = 'c'
            if 'class ' in code and '::' in code:
                language = 'cpp'
            elif 'public ' in code and 'void ' in code:
                language = 'csharp'

            examples.append({
                'chunk_index': i,
                'language': language,
                'code': code,
                'line_count': len(code.split('\n')),
                'title': chunk.get('heading'),
            })

    return examples


def ingest_page(url: str, topic_ids: List[int] = None) -> Dict:
    """Ingest a single page into the database."""
    print(f"Ingesting: {url}")

    page_data = fetch_page(url)
    if not page_data:
        return {'error': 'Failed to fetch page'}

    chunks = chunk_page(page_data['soup'])
    if not chunks:
        return {'error': 'No content extracted'}

    # Combine all content for full_content field
    full_content = '\n\n'.join(
        f"## {c['heading']}\n{c['content']}" if c['heading'] else c['content']
        for c in chunks
    )
    word_count = len(full_content.split())

    conn = get_conn()
    c = conn.cursor()

    # Insert or update page
    c.execute("""
        INSERT INTO pages (url, title, toc_path, full_content, word_count, last_fetched, last_updated)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(url) DO UPDATE SET
            title = excluded.title,
            toc_path = excluded.toc_path,
            full_content = excluded.full_content,
            word_count = excluded.word_count,
            last_fetched = excluded.last_fetched,
            last_updated = excluded.last_updated
    """, (
        url,
        page_data['title'],
        page_data['toc_path'],
        full_content,
        word_count,
        datetime.now().isoformat(),
        page_data['updated'],
    ))

    page_id = c.lastrowid or c.execute("SELECT id FROM pages WHERE url = ?", (url,)).fetchone()[0]

    # Link to topics
    if topic_ids:
        for topic_id in topic_ids:
            try:
                c.execute("INSERT INTO page_topics (page_id, topic_id) VALUES (?, ?)", (page_id, topic_id))
            except sqlite3.IntegrityError:
                pass

    # Clear existing concepts for this page (in case of re-ingest)
    c.execute("DELETE FROM concepts WHERE page_id = ?", (page_id,))

    # Insert concepts
    concept_ids = []
    for i, chunk in enumerate(chunks):
        keywords = ','.join(chunk.get('apis', []))

        c.execute("""
            INSERT INTO concepts (page_id, heading, content, word_count, chunk_index, concept_type, keywords)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (
            page_id,
            chunk['heading'],
            chunk['content'],
            chunk['word_count'],
            i,
            chunk['concept_type'],
            keywords,
        ))
        concept_ids.append(c.lastrowid)

        # Link APIs
        for api_name in chunk.get('apis', []):
            # Get or create API reference
            api_type = 'function' if api_name in KNOWN_APIS else 'structure'
            c.execute("""
                INSERT OR IGNORE INTO api_references (name, api_type, short_description)
                VALUES (?, ?, ?)
            """, (api_name, api_type, f"Referenced in {page_data['title']}"))

            api_id = c.execute("SELECT id FROM api_references WHERE name = ?", (api_name,)).fetchone()[0]

            try:
                c.execute("INSERT INTO concept_apis (concept_id, api_id) VALUES (?, ?)",
                          (c.lastrowid, api_id))
            except sqlite3.IntegrityError:
                pass

    # Insert code examples
    examples = extract_code_examples(chunks)
    for ex in examples:
        concept_id = concept_ids[ex['chunk_index']] if ex['chunk_index'] < len(concept_ids) else None
        c.execute("""
            INSERT INTO code_examples (page_id, concept_id, language, title, code, line_count)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (page_id, concept_id, ex['language'], ex['title'], ex['code'], ex['line_count']))

    conn.commit()
    conn.close()

    print(f"  ✓ {len(chunks)} concepts, {len(examples)} code examples, {word_count} words")

    return {
        'url': url,
        'title': page_data['title'],
        'concepts': len(chunks),
        'examples': len(examples),
        'word_count': word_count,
        'apis_found': list(set(api for chunk in chunks for api in chunk.get('apis', []))),
    }


def ingest_urls(urls: List[str], topic_names: List[str] = None) -> Dict:
    """Ingest multiple URLs."""
    conn = get_conn()
    c = conn.cursor()

    # Resolve topic names to IDs
    topic_ids = []
    if topic_names:
        for name in topic_names:
            row = c.execute("SELECT id FROM topics WHERE name = ?", (name,)).fetchone()
            if row:
                topic_ids.append(row[0])
    conn.close()

    results = []
    for url in urls:
        result = ingest_page(url, topic_ids)
        results.append(result)
        # Rate limiting
        import time
        time.sleep(0.5)

    return {
        'ingested': len([r for r in results if 'error' not in r]),
        'failed': len([r for r in results if 'error' in r]),
        'results': results,
    }


def main():
    parser = argparse.ArgumentParser(description='Ingest MS Learn pages into reference database')
    parser.add_argument('url', nargs='?', help='URL to ingest')
    parser.add_argument('--file', '-f', help='File containing URLs (one per line)')
    parser.add_argument('--topics', '-t', nargs='+', help='Topic names to associate with pages')

    args = parser.parse_args()

    if args.url:
        result = ingest_page(args.url)
        print(json.dumps(result, indent=2))
    elif args.file:
        with open(args.file) as f:
            urls = [line.strip() for line in f if line.strip() and not line.startswith('#')]
        result = ingest_urls(urls, args.topics)
        print(json.dumps(result, indent=2))
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
