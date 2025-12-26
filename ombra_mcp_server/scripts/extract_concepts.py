#!/usr/bin/env python3
"""
Extract concepts from all old project documentation.

Deduplicates concepts across files by merging VMCS fields, MSRs,
exit reasons, and tracking all source files.
"""

import sys
from pathlib import Path
from typing import Dict, Any

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_watcherd.concepts_db import ConceptsDB
from ombra_watcherd.extractors.doc_parser import extract_concepts_from_doc


OLD_DOCS_PATH = Path("/Users/jonathanmcclintock/PROJECT-OMBRA/docs/old_ombra_project")


def merge_concept(existing: Dict[str, Any], new: Dict[str, Any]) -> None:
    """Merge new concept data into existing concept."""
    # Merge list fields
    for field in ["vmcs_fields", "msrs", "exit_reasons"]:
        existing[field] = list(set(existing.get(field, []) + new.get(field, [])))

    # Track multiple source files
    sources = existing.get("source_doc", "")
    new_source = new.get("source_doc", "")
    if new_source and new_source not in sources:
        existing["source_doc"] = f"{sources}, {new_source}" if sources else new_source

    # Upgrade priority if new is higher
    priority_rank = {"critical": 1, "high": 2, "medium": 3, "low": 4}
    if priority_rank.get(new.get("priority"), 5) < priority_rank.get(existing.get("priority"), 5):
        existing["priority"] = new["priority"]


def main():
    db = ConceptsDB()

    # Find all markdown files
    md_files = sorted(OLD_DOCS_PATH.glob("*.md"))
    total_files = len(md_files)
    print(f"Found {total_files} markdown files")

    # Collect all concepts globally, deduplicating by ID
    global_concepts: Dict[str, Dict[str, Any]] = {}

    for idx, md_file in enumerate(md_files, 1):
        print(f"[{idx}/{total_files}] Processing: {md_file.name}")

        try:
            content = md_file.read_text(encoding="utf-8")
            concepts = extract_concepts_from_doc(content, md_file.name)

            for concept in concepts:
                cid = concept["id"]
                if cid in global_concepts:
                    merge_concept(global_concepts[cid], concept)
                else:
                    global_concepts[cid] = concept

        except Exception as e:
            print(f"  ERROR: {e}")

    # Add deduplicated concepts to database
    print(f"\n{'='*50}")
    print(f"Adding {len(global_concepts)} unique concepts to database...")

    for concept in global_concepts.values():
        db.add_concept(concept)
        vmcs = len(concept.get("vmcs_fields", []))
        msrs = len(concept.get("msrs", []))
        print(f"  + {concept['id']}: {concept['category']}, priority={concept['priority']}, vmcs={vmcs}, msrs={msrs}")

    print(f"\n{'='*50}")
    print(f"Total unique concepts: {len(global_concepts)}")

    # Summary by category
    for cat in ["timing", "vmx", "ept", "stealth"]:
        count = len(db.list_concepts(category=cat))
        print(f"  {cat}: {count}")


if __name__ == "__main__":
    main()
