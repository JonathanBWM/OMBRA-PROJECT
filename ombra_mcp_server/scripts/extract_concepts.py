#!/usr/bin/env python3
"""
Extract concepts from all old project documentation.
"""

import sys
from pathlib import Path

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_watcherd.concepts_db import ConceptsDB
from ombra_watcherd.extractors.doc_parser import extract_concepts_from_doc


OLD_DOCS_PATH = Path("/Users/jonathanmcclintock/PROJECT-OMBRA/docs/old_ombra_project")


def main():
    db = ConceptsDB()

    # Find all markdown files
    md_files = sorted(OLD_DOCS_PATH.glob("*.md"))
    total_files = len(md_files)
    print(f"Found {total_files} markdown files")

    total_concepts = 0

    for idx, md_file in enumerate(md_files, 1):
        print(f"\n[{idx}/{total_files}] Processing: {md_file.name}")

        try:
            content = md_file.read_text(encoding="utf-8")
            concepts = extract_concepts_from_doc(content, md_file.name)

            for concept in concepts:
                db.add_concept(concept)
                total_concepts += 1
                print(f"  + {concept['id']} ({concept['category']})")

        except Exception as e:
            print(f"  ERROR: {e}")

    print(f"\n{'='*50}")
    print(f"Total concepts extracted: {total_concepts}")

    # Summary by category
    for cat in ["timing", "vmx", "ept", "stealth"]:
        count = len(db.list_concepts(category=cat))
        print(f"  {cat}: {count}")


if __name__ == "__main__":
    main()
