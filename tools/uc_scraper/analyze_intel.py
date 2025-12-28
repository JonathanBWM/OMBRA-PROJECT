#!/usr/bin/env python3
"""
Intel Analysis Tool
Analyze scraped UnknownCheats data for anti-cheat research

Generates reports on:
- Detection techniques mentioned
- Evasion strategies discussed
- Code patterns and implementations
- Popular tools and methods
"""

import sqlite3
import json
import re
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import List, Dict, Optional
import argparse


@dataclass
class IntelReport:
    """Analysis report structure"""
    detection_vectors: Dict[str, int]
    evasion_techniques: Dict[str, int]
    code_snippets: List[dict]
    author_expertise: Dict[str, int]
    tool_mentions: Dict[str, int]
    anticheat_mentions: Dict[str, int]
    hot_topics: List[dict]


class IntelAnalyzer:
    """Analyze scraped forum data for actionable intelligence"""

    # Detection vector keywords
    DETECTION_KEYWORDS = {
        # Pool/Memory Detection
        "bigpool": "BigPool Scanning",
        "poolbigpagetable": "BigPool Table",
        "pool tag": "Pool Tag Detection",
        "mmallocatepages": "MmAllocate Detection",
        "nonpagedpool": "NonPagedPool Scanning",
        "executablepool": "Executable Pool",

        # PE/Module Detection
        "pe header": "PE Header Scanning",
        "mz signature": "MZ Signature",
        "dos header": "DOS Header Detection",
        "psloadedmodulelist": "Loaded Module List",
        "mmunloadeddrivers": "Unloaded Drivers",
        "piddbcachetable": "PiDDB Cache",

        # Timing Detection
        "rdtsc": "RDTSC Timing",
        "timing attack": "Timing Analysis",
        "tsc offset": "TSC Offset Detection",

        # Hypervisor Detection
        "cpuid hypervisor": "CPUID Hypervisor Bit",
        "vmx": "VMX Detection",
        "ept": "EPT Detection",
        "vmexit": "VMExit Detection",
        "hypervisor present": "Hypervisor Presence",

        # ETW/Callbacks
        "etw": "ETW Tracing",
        "pssetcreateprocess": "Process Callbacks",
        "pssetloadimage": "Image Load Callbacks",
        "obregistercallbacks": "Object Callbacks",

        # Integrity
        "patchguard": "PatchGuard",
        "hvci": "HVCI",
        "dse": "DSE",
        "secureboot": "Secure Boot",
    }

    # Evasion technique keywords
    EVASION_KEYWORDS = {
        # Memory
        "mdl": "MDL Allocation",
        "mmallocatepagesformdl": "MmAllocatePagesForMdl",
        "physical memory": "Physical Memory",
        "page table": "Page Table Manipulation",

        # Mapping
        "manual map": "Manual Mapping",
        "kdmapper": "KDMapper",
        "drvmap": "Driver Mapper",

        # Signature
        "signature wipe": "Signature Wiping",
        "pe wipe": "PE Header Wiping",
        "pool tag spoof": "Pool Tag Spoofing",

        # Hypervisor
        "hypervisor": "Hypervisor-based",
        "ept hook": "EPT Hooking",
        "shadow": "Shadow Techniques",
        "type 1": "Type-1 Hypervisor",

        # BYOVD
        "byovd": "BYOVD",
        "vulnerable driver": "Vulnerable Driver",
        "ld9boxsup": "LDPlayer Driver",
        "capcom": "Capcom Driver",
        "gdrv": "Gigabyte Driver",

        # Cleanup
        "trace clean": "Trace Cleanup",
        "log clean": "Log Cleanup",
        "etw bypass": "ETW Bypass",
    }

    # Anti-cheat systems
    ANTICHEATS = [
        "easyanticheat", "eac",
        "battleye", "be",
        "vanguard",
        "faceit",
        "esea",
        "vac",
        "mrac",
        "xigncode",
        "nprotect",
        "gameguard",
        "ricochet",
    ]

    # Tools
    TOOLS = [
        "kdmapper", "kdstinker", "drvmap",
        "cheat engine", "reclass",
        "ida", "ghidra", "x64dbg",
        "procmon", "windbg",
        "process hacker",
        "vmware", "virtualbox", "hyper-v",
    ]

    def __init__(self, db_path: str = "uc_intel.db"):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)

    def analyze(self) -> IntelReport:
        """Run full analysis"""
        posts = self._get_all_posts()

        return IntelReport(
            detection_vectors=self._count_keywords(posts, self.DETECTION_KEYWORDS),
            evasion_techniques=self._count_keywords(posts, self.EVASION_KEYWORDS),
            code_snippets=self._extract_code_snippets(posts),
            author_expertise=self._analyze_authors(),
            tool_mentions=self._count_simple_keywords(posts, self.TOOLS),
            anticheat_mentions=self._count_simple_keywords(posts, self.ANTICHEATS),
            hot_topics=self._find_hot_topics()
        )

    def _get_all_posts(self) -> List[dict]:
        """Get all posts from database"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT p.post_id, p.thread_id, p.author, p.content, p.code_blocks, t.title, t.views
            FROM posts p
            JOIN threads t ON p.thread_id = t.thread_id
        """)

        return [{
            "post_id": r[0],
            "thread_id": r[1],
            "author": r[2],
            "content": r[3].lower() if r[3] else "",
            "code_blocks": json.loads(r[4]) if r[4] else [],
            "thread_title": r[5],
            "views": r[6]
        } for r in cursor.fetchall()]

    def _count_keywords(self, posts: List[dict], keyword_map: Dict[str, str]) -> Dict[str, int]:
        """Count occurrences of keywords"""
        counts = Counter()

        for post in posts:
            content = post["content"]
            for keyword, label in keyword_map.items():
                if keyword in content:
                    counts[label] += 1

        return dict(counts.most_common(30))

    def _count_simple_keywords(self, posts: List[dict], keywords: List[str]) -> Dict[str, int]:
        """Count simple keyword occurrences"""
        counts = Counter()

        for post in posts:
            content = post["content"]
            for keyword in keywords:
                if keyword in content:
                    counts[keyword] += 1

        return dict(counts.most_common(20))

    def _extract_code_snippets(self, posts: List[dict], max_snippets: int = 50) -> List[dict]:
        """Extract and categorize code snippets"""
        snippets = []

        for post in posts:
            for code in post.get("code_blocks", []):
                if len(code) < 50:  # Too short
                    continue
                if len(code) > 5000:  # Too long
                    continue

                # Categorize the snippet
                category = self._categorize_code(code)

                snippets.append({
                    "code": code[:1000],  # Truncate for storage
                    "category": category,
                    "author": post["author"],
                    "thread": post["thread_title"],
                    "views": post["views"]
                })

        # Sort by thread views and return top snippets
        snippets.sort(key=lambda x: x["views"], reverse=True)
        return snippets[:max_snippets]

    def _categorize_code(self, code: str) -> str:
        """Categorize a code snippet by content"""
        code_lower = code.lower()

        categories = [
            (["vmread", "vmwrite", "vmcs", "vmlaunch"], "VMCS/VMX"),
            (["ept", "pml4", "pdpt", "pde", "pte"], "EPT"),
            (["mdl", "mmallocate", "mmmap"], "Memory Allocation"),
            (["ntquerysystem", "zwquerysystem"], "System Information"),
            (["createfile", "deviceiocontrol"], "Driver Communication"),
            (["iocreatedevice", "driverobject"], "Driver Development"),
            (["rdmsr", "wrmsr", "msr"], "MSR Access"),
            (["hook", "detour", "trampoline"], "Hooking"),
            (["signature", "pattern", "scan"], "Pattern Scanning"),
            (["encrypt", "decrypt", "xor"], "Encryption"),
        ]

        for keywords, category in categories:
            if any(kw in code_lower for kw in keywords):
                return category

        return "General"

    def _analyze_authors(self) -> Dict[str, int]:
        """Find authors with most valuable contributions"""
        cursor = self.conn.cursor()

        # Count posts per author weighted by thread views
        cursor.execute("""
            SELECT p.author, COUNT(*) as post_count, SUM(t.views) as total_views
            FROM posts p
            JOIN threads t ON p.thread_id = t.thread_id
            GROUP BY p.author
            ORDER BY total_views DESC
            LIMIT 30
        """)

        return {r[0]: r[2] for r in cursor.fetchall()}

    def _find_hot_topics(self) -> List[dict]:
        """Find trending topics based on views and replies"""
        cursor = self.conn.cursor()

        cursor.execute("""
            SELECT thread_id, title, url, author, replies, views
            FROM threads
            ORDER BY views DESC
            LIMIT 30
        """)

        return [{
            "title": r[1],
            "url": r[2],
            "author": r[3],
            "replies": r[4],
            "views": r[5]
        } for r in cursor.fetchall()]

    def generate_report(self, output_path: str = "intel_report.md"):
        """Generate markdown report"""
        report = self.analyze()

        lines = [
            "# UnknownCheats Anti-Cheat Bypass Intelligence Report",
            "",
            f"*Generated from scraped forum data*",
            "",
            "## Detection Vectors Discussed",
            "",
            "| Technique | Mentions |",
            "|-----------|----------|",
        ]

        for technique, count in report.detection_vectors.items():
            lines.append(f"| {technique} | {count} |")

        lines.extend([
            "",
            "## Evasion Techniques Discussed",
            "",
            "| Technique | Mentions |",
            "|-----------|----------|",
        ])

        for technique, count in report.evasion_techniques.items():
            lines.append(f"| {technique} | {count} |")

        lines.extend([
            "",
            "## Anti-Cheat Systems Discussed",
            "",
            "| Anti-Cheat | Mentions |",
            "|------------|----------|",
        ])

        for ac, count in report.anticheat_mentions.items():
            lines.append(f"| {ac} | {count} |")

        lines.extend([
            "",
            "## Tools Mentioned",
            "",
            "| Tool | Mentions |",
            "|------|----------|",
        ])

        for tool, count in report.tool_mentions.items():
            lines.append(f"| {tool} | {count} |")

        lines.extend([
            "",
            "## Hot Topics (by views)",
            "",
        ])

        for topic in report.hot_topics[:15]:
            lines.append(f"- **{topic['title'][:60]}** ({topic['views']:,} views, {topic['replies']} replies)")

        lines.extend([
            "",
            "## Expert Contributors",
            "",
            "Top contributors by total thread views of their posts:",
            "",
        ])

        for author, views in list(report.author_expertise.items())[:15]:
            lines.append(f"- **{author}**: {views:,} total views")

        lines.extend([
            "",
            "## Code Snippet Categories",
            "",
        ])

        category_counts = Counter(s["category"] for s in report.code_snippets)
        for category, count in category_counts.most_common():
            lines.append(f"- {category}: {count} snippets")

        # Write report
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))

        print(f"Report written to {output_path}")
        return output_path

    def search_context(self, keyword: str, context_chars: int = 200) -> List[dict]:
        """Search for keyword with surrounding context"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT p.content, p.author, t.title, t.url
            FROM posts p
            JOIN threads t ON p.thread_id = t.thread_id
            WHERE p.content LIKE ?
        """, (f"%{keyword}%",))

        results = []
        for row in cursor.fetchall():
            content = row[0] or ""
            idx = content.lower().find(keyword.lower())
            if idx == -1:
                continue

            start = max(0, idx - context_chars)
            end = min(len(content), idx + len(keyword) + context_chars)
            context = content[start:end]

            results.append({
                "context": f"...{context}...",
                "author": row[1],
                "thread": row[2],
                "url": row[3]
            })

        return results[:20]

    def close(self):
        self.conn.close()


def main():
    parser = argparse.ArgumentParser(description="Analyze scraped UC intel")
    parser.add_argument("--db", default="uc_intel.db", help="Database path")
    parser.add_argument("--report", action="store_true", help="Generate full report")
    parser.add_argument("--search", type=str, help="Search for keyword with context")
    parser.add_argument("--output", default="intel_report.md", help="Report output path")

    args = parser.parse_args()

    if not Path(args.db).exists():
        print(f"Database not found: {args.db}")
        print("Run the scraper first: python scraper.py")
        return

    analyzer = IntelAnalyzer(args.db)

    try:
        if args.search:
            results = analyzer.search_context(args.search)
            print(f"\nFound {len(results)} results for '{args.search}':\n")
            for r in results:
                print(f"[{r['thread'][:50]}] by {r['author']}")
                print(f"  {r['context']}")
                print(f"  URL: {r['url']}\n")

        elif args.report:
            analyzer.generate_report(args.output)

        else:
            # Quick summary
            report = analyzer.analyze()
            print("\n=== Quick Summary ===\n")

            print("Top Detection Vectors:")
            for tech, count in list(report.detection_vectors.items())[:5]:
                print(f"  {tech}: {count}")

            print("\nTop Evasion Techniques:")
            for tech, count in list(report.evasion_techniques.items())[:5]:
                print(f"  {tech}: {count}")

            print("\nTop Anti-Cheats Discussed:")
            for ac, count in list(report.anticheat_mentions.items())[:5]:
                print(f"  {ac}: {count}")

    finally:
        analyzer.close()


if __name__ == "__main__":
    main()
