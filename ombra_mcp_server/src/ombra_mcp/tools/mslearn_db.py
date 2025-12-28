"""
MS Learn Reference Database Tools

Query Windows kernel documentation with semantic search, API lookups,
and project-specific annotations.

Tools:
- mslearn_search: Semantic search across all concepts
- mslearn_api: Get API reference details
- mslearn_concept: Get a specific concept by ID or search
- mslearn_page: Get full page content
- mslearn_examples: Get code examples
- mslearn_topics: List available topics
- mslearn_annotate: Add our notes to concepts/APIs
"""

import sqlite3
from pathlib import Path
from typing import List, Dict, Any, Optional
import json

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "mslearn_reference.db"


def _get_conn():
    """Get database connection."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


# ============================================================================
# Query Tools
# ============================================================================

def mslearn_search(
    query: str,
    topic: Optional[str] = None,
    concept_type: Optional[str] = None,
    limit: int = 10,
) -> Dict[str, Any]:
    """
    Semantic search across MS Learn concepts.

    Uses FTS5 for fast full-text search across all ingested documentation.

    Args:
        query: Search query (natural language or API names)
        topic: Filter to specific topic (e.g., 'MDL', 'Physical Memory')
        concept_type: Filter by type ('overview', 'usage', 'warning', 'example', 'api_list')
        limit: Maximum results to return

    Returns:
        List of matching concepts with relevance scores
    """
    conn = _get_conn()
    c = conn.cursor()

    # Build FTS query
    # Handle common search patterns
    fts_query = query.replace(' ', ' OR ')

    try:
        if topic:
            # Filter by topic
            c.execute("""
                SELECT c.*, p.title as page_title, p.url,
                       bm25(concepts_fts) as relevance
                FROM concepts_fts
                JOIN concepts c ON concepts_fts.rowid = c.id
                JOIN pages p ON c.page_id = p.id
                JOIN page_topics pt ON p.id = pt.page_id
                JOIN topics t ON pt.topic_id = t.id
                WHERE concepts_fts MATCH ?
                  AND t.name = ?
                  AND (? IS NULL OR c.concept_type = ?)
                ORDER BY relevance
                LIMIT ?
            """, (fts_query, topic, concept_type, concept_type, limit))
        else:
            c.execute("""
                SELECT c.*, p.title as page_title, p.url,
                       bm25(concepts_fts) as relevance
                FROM concepts_fts
                JOIN concepts c ON concepts_fts.rowid = c.id
                JOIN pages p ON c.page_id = p.id
                WHERE concepts_fts MATCH ?
                  AND (? IS NULL OR c.concept_type = ?)
                ORDER BY relevance
                LIMIT ?
            """, (fts_query, concept_type, concept_type, limit))

        results = []
        for row in c.fetchall():
            results.append({
                'id': row['id'],
                'heading': row['heading'],
                'content': row['content'][:500] + '...' if len(row['content']) > 500 else row['content'],
                'word_count': row['word_count'],
                'concept_type': row['concept_type'],
                'apis': row['keywords'].split(',') if row['keywords'] else [],
                'page_title': row['page_title'],
                'page_url': row['url'],
                'relevance': row['relevance'],
            })

        conn.close()
        return {
            'query': query,
            'results': results,
            'count': len(results),
        }

    except sqlite3.OperationalError as e:
        conn.close()
        # Fallback to LIKE search if FTS fails
        return mslearn_search_fallback(query, topic, concept_type, limit)


def mslearn_search_fallback(
    query: str,
    topic: Optional[str] = None,
    concept_type: Optional[str] = None,
    limit: int = 10,
) -> Dict[str, Any]:
    """Fallback LIKE-based search when FTS fails."""
    conn = _get_conn()
    c = conn.cursor()

    like_query = f"%{query}%"

    c.execute("""
        SELECT c.*, p.title as page_title, p.url
        FROM concepts c
        JOIN pages p ON c.page_id = p.id
        WHERE (c.content LIKE ? OR c.heading LIKE ? OR c.keywords LIKE ?)
          AND (? IS NULL OR c.concept_type = ?)
        LIMIT ?
    """, (like_query, like_query, like_query, concept_type, concept_type, limit))

    results = []
    for row in c.fetchall():
        results.append({
            'id': row['id'],
            'heading': row['heading'],
            'content': row['content'][:500] + '...' if len(row['content']) > 500 else row['content'],
            'word_count': row['word_count'],
            'concept_type': row['concept_type'],
            'apis': row['keywords'].split(',') if row['keywords'] else [],
            'page_title': row['page_title'],
            'page_url': row['url'],
        })

    conn.close()
    return {
        'query': query,
        'results': results,
        'count': len(results),
    }


def mslearn_api(name: str) -> Dict[str, Any]:
    """
    Get API reference details.

    Returns full information about a Windows kernel API including:
    - Type (function, macro, structure)
    - Parameters and return value
    - MS remarks
    - Our project-specific notes
    - Related concepts that mention this API

    Args:
        name: API name (e.g., 'MmGetMdlPfnArray', 'IoAllocateMdl')

    Returns:
        API details and related concepts
    """
    conn = _get_conn()
    c = conn.cursor()

    # Get API reference
    c.execute("""
        SELECT * FROM api_references
        WHERE name = ? OR name LIKE ?
    """, (name, f"%{name}%"))

    row = c.fetchone()
    if not row:
        conn.close()
        return {'error': f'API not found: {name}'}

    api = dict(row)

    # Parse JSON fields
    if api.get('parameters'):
        try:
            api['parameters'] = json.loads(api['parameters'])
        except:
            pass

    if api.get('related_apis'):
        try:
            api['related_apis'] = json.loads(api['related_apis'])
        except:
            pass

    # Get concepts that mention this API
    c.execute("""
        SELECT c.id, c.heading, c.content, c.concept_type, p.title as page_title, p.url
        FROM concept_apis ca
        JOIN concepts c ON ca.concept_id = c.id
        JOIN pages p ON c.page_id = p.id
        WHERE ca.api_id = ?
    """, (api['id'],))

    api['mentioned_in'] = []
    for row in c.fetchall():
        api['mentioned_in'].append({
            'concept_id': row['id'],
            'heading': row['heading'],
            'snippet': row['content'][:200] + '...',
            'page_title': row['page_title'],
            'page_url': row['url'],
        })

    # Also find concepts that mention this API in keywords but aren't linked
    c.execute("""
        SELECT c.id, c.heading, c.content, c.concept_type, p.title as page_title, p.url
        FROM concepts c
        JOIN pages p ON c.page_id = p.id
        WHERE c.keywords LIKE ?
        LIMIT 10
    """, (f"%{name}%",))

    for row in c.fetchall():
        if not any(m['concept_id'] == row['id'] for m in api['mentioned_in']):
            api['mentioned_in'].append({
                'concept_id': row['id'],
                'heading': row['heading'],
                'snippet': row['content'][:200] + '...',
                'page_title': row['page_title'],
                'page_url': row['url'],
            })

    conn.close()
    return api


def mslearn_concept(concept_id: int) -> Dict[str, Any]:
    """
    Get full concept by ID.

    Returns the complete concept content, associated code examples,
    and linked APIs.

    Args:
        concept_id: Concept ID from search results

    Returns:
        Full concept with code examples and API links
    """
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT c.*, p.title as page_title, p.url
        FROM concepts c
        JOIN pages p ON c.page_id = p.id
        WHERE c.id = ?
    """, (concept_id,))

    row = c.fetchone()
    if not row:
        conn.close()
        return {'error': f'Concept not found: {concept_id}'}

    concept = dict(row)

    # Get code examples
    c.execute("""
        SELECT * FROM code_examples
        WHERE concept_id = ?
    """, (concept_id,))
    concept['code_examples'] = [dict(r) for r in c.fetchall()]

    # Get linked APIs
    c.execute("""
        SELECT a.* FROM concept_apis ca
        JOIN api_references a ON ca.api_id = a.id
        WHERE ca.concept_id = ?
    """, (concept_id,))
    concept['apis'] = [dict(r) for r in c.fetchall()]

    conn.close()
    return concept


def mslearn_page(url_or_id: str) -> Dict[str, Any]:
    """
    Get full page content.

    Returns the complete page with all concepts, for when you need
    full context rather than a specific chunk.

    Args:
        url_or_id: Page URL or numeric ID

    Returns:
        Full page content with all concepts and examples
    """
    conn = _get_conn()
    c = conn.cursor()

    if url_or_id.isdigit():
        c.execute("SELECT * FROM pages WHERE id = ?", (int(url_or_id),))
    else:
        c.execute("SELECT * FROM pages WHERE url LIKE ?", (f"%{url_or_id}%",))

    row = c.fetchone()
    if not row:
        conn.close()
        return {'error': f'Page not found: {url_or_id}'}

    page = dict(row)
    page_id = page['id']

    # Get all concepts
    c.execute("""
        SELECT * FROM concepts
        WHERE page_id = ?
        ORDER BY chunk_index
    """, (page_id,))
    page['concepts'] = [dict(r) for r in c.fetchall()]

    # Get all code examples
    c.execute("""
        SELECT * FROM code_examples
        WHERE page_id = ?
    """, (page_id,))
    page['code_examples'] = [dict(r) for r in c.fetchall()]

    # Get topics
    c.execute("""
        SELECT t.name FROM page_topics pt
        JOIN topics t ON pt.topic_id = t.id
        WHERE pt.page_id = ?
    """, (page_id,))
    page['topics'] = [r['name'] for r in c.fetchall()]

    conn.close()
    return page


def mslearn_examples(
    topic: Optional[str] = None,
    api_name: Optional[str] = None,
    limit: int = 10,
) -> Dict[str, Any]:
    """
    Get code examples, optionally filtered by topic or API.

    Args:
        topic: Filter by topic (e.g., 'MDL')
        api_name: Filter by API name mentioned in the concept
        limit: Maximum examples to return

    Returns:
        List of code examples with context
    """
    conn = _get_conn()
    c = conn.cursor()

    if api_name:
        c.execute("""
            SELECT e.*, c.heading as concept_heading, p.title as page_title, p.url
            FROM code_examples e
            JOIN concepts c ON e.concept_id = c.id
            JOIN pages p ON e.page_id = p.id
            WHERE c.keywords LIKE ?
            LIMIT ?
        """, (f"%{api_name}%", limit))
    elif topic:
        c.execute("""
            SELECT e.*, c.heading as concept_heading, p.title as page_title, p.url
            FROM code_examples e
            JOIN concepts c ON e.concept_id = c.id
            JOIN pages p ON e.page_id = p.id
            JOIN page_topics pt ON p.id = pt.page_id
            JOIN topics t ON pt.topic_id = t.id
            WHERE t.name = ?
            LIMIT ?
        """, (topic, limit))
    else:
        c.execute("""
            SELECT e.*, c.heading as concept_heading, p.title as page_title, p.url
            FROM code_examples e
            JOIN concepts c ON e.concept_id = c.id
            JOIN pages p ON e.page_id = p.id
            LIMIT ?
        """, (limit,))

    results = [dict(r) for r in c.fetchall()]
    conn.close()

    return {
        'topic': topic,
        'api_name': api_name,
        'examples': results,
        'count': len(results),
    }


def mslearn_topics() -> Dict[str, Any]:
    """
    List all topics with page counts.

    Returns:
        List of topics with descriptions and relevance to our project
    """
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        SELECT t.*, COUNT(pt.page_id) as page_count
        FROM topics t
        LEFT JOIN page_topics pt ON t.id = pt.topic_id
        GROUP BY t.id
        ORDER BY t.priority DESC, t.name
    """)

    topics = [dict(r) for r in c.fetchall()]
    conn.close()

    return {'topics': topics}


# ============================================================================
# Annotation Tools
# ============================================================================

def mslearn_annotate_concept(
    concept_id: int,
    notes: str,
) -> Dict[str, Any]:
    """
    Add our notes to a concept.

    Args:
        concept_id: Concept ID
        notes: Our project-specific notes

    Returns:
        Updated concept
    """
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        UPDATE concepts SET our_notes = ?
        WHERE id = ?
    """, (notes, concept_id))

    conn.commit()
    conn.close()

    return mslearn_concept(concept_id)


def mslearn_annotate_api(
    api_name: str,
    notes: str,
) -> Dict[str, Any]:
    """
    Add our notes to an API.

    Args:
        api_name: API name
        notes: Our project-specific notes (gotchas, tips, usage patterns)

    Returns:
        Updated API reference
    """
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        UPDATE api_references SET our_notes = ?
        WHERE name = ?
    """, (notes, api_name))

    if c.rowcount == 0:
        conn.close()
        return {'error': f'API not found: {api_name}'}

    conn.commit()
    conn.close()

    return mslearn_api(api_name)


def mslearn_cross_reference(
    source_type: str,  # 'api', 'concept', 'example'
    source_id: int,
    target_db: str,    # 'project_brain', 'evasion_techniques', etc.
    target_type: str,  # 'gotcha', 'technique', 'detection'
    target_id: str,
    relationship: str,  # 'relates_to', 'warns_about', 'enables', 'detected_by'
    notes: Optional[str] = None,
) -> Dict[str, Any]:
    """
    Create a cross-reference between MS Learn content and our other databases.

    Args:
        source_type: 'api', 'concept', or 'example'
        source_id: ID in mslearn_reference.db
        target_db: Target database name
        target_type: Type of target record
        target_id: ID in target database
        relationship: Type of relationship
        notes: Optional notes about the relationship

    Returns:
        Created cross-reference
    """
    conn = _get_conn()
    c = conn.cursor()

    c.execute("""
        INSERT INTO cross_references
        (source_type, source_id, target_db, target_type, target_id, relationship, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """, (source_type, source_id, target_db, target_type, target_id, relationship, notes))

    xref_id = c.lastrowid
    conn.commit()
    conn.close()

    return {
        'id': xref_id,
        'source_type': source_type,
        'source_id': source_id,
        'target_db': target_db,
        'target_type': target_type,
        'target_id': target_id,
        'relationship': relationship,
    }


# ============================================================================
# Stats
# ============================================================================

def mslearn_stats() -> Dict[str, Any]:
    """
    Get database statistics.

    Returns:
        Counts of pages, concepts, APIs, examples
    """
    conn = _get_conn()
    c = conn.cursor()

    stats = {}

    c.execute("SELECT COUNT(*) FROM pages")
    stats['pages'] = c.fetchone()[0]

    c.execute("SELECT COUNT(*) FROM concepts")
    stats['concepts'] = c.fetchone()[0]

    c.execute("SELECT COUNT(*) FROM api_references")
    stats['apis'] = c.fetchone()[0]

    c.execute("SELECT COUNT(*) FROM code_examples")
    stats['examples'] = c.fetchone()[0]

    c.execute("SELECT SUM(word_count) FROM pages")
    stats['total_words'] = c.fetchone()[0] or 0

    c.execute("SELECT COUNT(*) FROM cross_references")
    stats['cross_references'] = c.fetchone()[0]

    # Concept types breakdown
    c.execute("""
        SELECT concept_type, COUNT(*) as count
        FROM concepts
        GROUP BY concept_type
    """)
    stats['concept_types'] = {r['concept_type']: r['count'] for r in c.fetchall()}

    conn.close()
    return stats
