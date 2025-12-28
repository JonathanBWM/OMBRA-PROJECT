#!/usr/bin/env python3
"""
UnknownCheats Forum Scraper
Research intelligence gathering for anti-cheat bypass techniques

Uses Selenium with undetected-chromedriver to bypass Cloudflare protection.
Implements respectful rate limiting and session persistence.
"""

import os
import sys
import json
import time
import random
import hashlib
import sqlite3
import logging
from datetime import datetime
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import Optional, List, Generator
from urllib.parse import urljoin, urlparse, parse_qs

try:
    import undetected_chromedriver as uc
    from selenium.webdriver.common.by import By
    from selenium.webdriver.support.ui import WebDriverWait
    from selenium.webdriver.support import expected_conditions as EC
    from selenium.common.exceptions import TimeoutException, NoSuchElementException
    from bs4 import BeautifulSoup
    import requests
except ImportError as e:
    print(f"Missing dependency: {e}")
    print("Install with: pip install undetected-chromedriver selenium beautifulsoup4 requests")
    sys.exit(1)

# Configuration
BASE_URL = "https://www.unknowncheats.me"
FORUM_URL = f"{BASE_URL}/forum/anti-cheat-bypass/"
USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

# Rate limiting (be respectful)
MIN_DELAY = 3.0  # Minimum seconds between requests
MAX_DELAY = 8.0  # Maximum seconds between requests
PAGE_DELAY = 5.0  # Extra delay between pages

# Logging setup
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('scraper.log')
    ]
)
log = logging.getLogger(__name__)


@dataclass
class ForumThread:
    """Represents a forum thread"""
    thread_id: str
    title: str
    url: str
    author: str
    replies: int
    views: int
    last_post_date: str
    subforum: str
    scraped_at: str


@dataclass
class ForumPost:
    """Represents a single post within a thread"""
    post_id: str
    thread_id: str
    author: str
    author_reputation: Optional[str]
    content: str
    post_date: str
    is_op: bool
    code_blocks: List[str]
    attachments: List[str]
    scraped_at: str


class Database:
    """SQLite database for storing scraped data"""

    def __init__(self, db_path: str = "uc_intel.db"):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self._init_schema()

    def _init_schema(self):
        """Initialize database schema"""
        cursor = self.conn.cursor()

        cursor.execute("""
            CREATE TABLE IF NOT EXISTS threads (
                thread_id TEXT PRIMARY KEY,
                title TEXT,
                url TEXT,
                author TEXT,
                replies INTEGER,
                views INTEGER,
                last_post_date TEXT,
                subforum TEXT,
                scraped_at TEXT,
                content_scraped INTEGER DEFAULT 0
            )
        """)

        cursor.execute("""
            CREATE TABLE IF NOT EXISTS posts (
                post_id TEXT PRIMARY KEY,
                thread_id TEXT,
                author TEXT,
                author_reputation TEXT,
                content TEXT,
                post_date TEXT,
                is_op INTEGER,
                code_blocks TEXT,
                attachments TEXT,
                scraped_at TEXT,
                FOREIGN KEY (thread_id) REFERENCES threads(thread_id)
            )
        """)

        cursor.execute("""
            CREATE TABLE IF NOT EXISTS keywords (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                keyword TEXT UNIQUE,
                category TEXT,
                priority INTEGER DEFAULT 0
            )
        """)

        # Insert default keywords for anti-cheat research
        keywords = [
            ("BigPool", "detection", 10),
            ("PoolBigPageTable", "detection", 10),
            ("MmAllocatePagesForMdl", "evasion", 9),
            ("PageAllocEx", "detection", 8),
            ("EPT", "hypervisor", 9),
            ("VMCS", "hypervisor", 9),
            ("hypervisor", "hypervisor", 10),
            ("EasyAntiCheat", "anticheat", 10),
            ("BattlEye", "anticheat", 10),
            ("Vanguard", "anticheat", 10),
            ("manual map", "evasion", 8),
            ("KDMapper", "tools", 7),
            ("BYOVD", "evasion", 9),
            ("PE header", "detection", 8),
            ("pool tag", "detection", 8),
            ("NtQuerySystemInformation", "api", 7),
            ("SystemBigPoolInformation", "detection", 9),
            ("ETW", "detection", 8),
            ("PatchGuard", "protection", 8),
            ("DSE", "protection", 7),
            ("HVCI", "protection", 9),
            ("VBS", "protection", 8),
            ("timing attack", "detection", 7),
            ("RDTSC", "detection", 8),
            ("CPUID", "detection", 7),
            ("VirtualBox", "hypervisor", 6),
            ("VMware", "hypervisor", 6),
            ("SUPDrv", "driver", 8),
            ("IoCreateDevice", "api", 6),
            ("PsLoadedModuleList", "detection", 8),
            ("MmUnloadedDrivers", "detection", 8),
            ("PiDDBCacheTable", "detection", 9),
        ]

        for keyword, category, priority in keywords:
            try:
                cursor.execute(
                    "INSERT OR IGNORE INTO keywords (keyword, category, priority) VALUES (?, ?, ?)",
                    (keyword, category, priority)
                )
            except sqlite3.IntegrityError:
                pass

        cursor.execute("""
            CREATE INDEX IF NOT EXISTS idx_posts_thread ON posts(thread_id)
        """)

        cursor.execute("""
            CREATE VIRTUAL TABLE IF NOT EXISTS posts_fts USING fts5(
                content,
                content_rowid=post_id
            )
        """)

        self.conn.commit()

    def save_thread(self, thread: ForumThread):
        """Save or update a thread"""
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT OR REPLACE INTO threads
            (thread_id, title, url, author, replies, views, last_post_date, subforum, scraped_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            thread.thread_id, thread.title, thread.url, thread.author,
            thread.replies, thread.views, thread.last_post_date,
            thread.subforum, thread.scraped_at
        ))
        self.conn.commit()

    def save_post(self, post: ForumPost):
        """Save or update a post"""
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT OR REPLACE INTO posts
            (post_id, thread_id, author, author_reputation, content, post_date,
             is_op, code_blocks, attachments, scraped_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            post.post_id, post.thread_id, post.author, post.author_reputation,
            post.content, post.post_date, int(post.is_op),
            json.dumps(post.code_blocks), json.dumps(post.attachments),
            post.scraped_at
        ))

        # Update FTS index
        try:
            cursor.execute("""
                INSERT OR REPLACE INTO posts_fts(rowid, content)
                VALUES (?, ?)
            """, (hash(post.post_id) & 0x7FFFFFFF, post.content))
        except:
            pass  # FTS might fail, not critical

        self.conn.commit()

    def mark_thread_scraped(self, thread_id: str):
        """Mark a thread's content as scraped"""
        cursor = self.conn.cursor()
        cursor.execute(
            "UPDATE threads SET content_scraped = 1 WHERE thread_id = ?",
            (thread_id,)
        )
        self.conn.commit()

    def get_unscraped_threads(self, limit: int = 100) -> List[dict]:
        """Get threads that haven't had their content scraped yet"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT thread_id, title, url FROM threads
            WHERE content_scraped = 0
            ORDER BY views DESC
            LIMIT ?
        """, (limit,))
        return [{"thread_id": r[0], "title": r[1], "url": r[2]} for r in cursor.fetchall()]

    def search(self, query: str, limit: int = 50) -> List[dict]:
        """Full-text search across posts"""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT p.thread_id, p.author, p.content, t.title, t.url
            FROM posts p
            JOIN threads t ON p.thread_id = t.thread_id
            WHERE p.content LIKE ?
            ORDER BY t.views DESC
            LIMIT ?
        """, (f"%{query}%", limit))

        return [{
            "thread_id": r[0],
            "author": r[1],
            "content": r[2][:500],
            "thread_title": r[3],
            "url": r[4]
        } for r in cursor.fetchall()]

    def get_stats(self) -> dict:
        """Get database statistics"""
        cursor = self.conn.cursor()

        cursor.execute("SELECT COUNT(*) FROM threads")
        thread_count = cursor.fetchone()[0]

        cursor.execute("SELECT COUNT(*) FROM posts")
        post_count = cursor.fetchone()[0]

        cursor.execute("SELECT COUNT(*) FROM threads WHERE content_scraped = 1")
        scraped_count = cursor.fetchone()[0]

        return {
            "threads": thread_count,
            "posts": post_count,
            "threads_with_content": scraped_count
        }

    def close(self):
        self.conn.close()


class UCScraper:
    """Main scraper class using Selenium with undetected-chromedriver"""

    def __init__(self, headless: bool = True, db_path: str = "uc_intel.db"):
        self.db = Database(db_path)
        self.driver = None
        self.headless = headless
        self._session_cookies = None

    def _init_driver(self):
        """Initialize undetected Chrome driver"""
        if self.driver:
            return

        log.info("Initializing Chrome driver...")

        options = uc.ChromeOptions()
        if self.headless:
            options.add_argument('--headless=new')

        options.add_argument('--no-sandbox')
        options.add_argument('--disable-dev-shm-usage')
        options.add_argument('--disable-gpu')
        options.add_argument(f'--user-agent={USER_AGENT}')
        options.add_argument('--window-size=1920,1080')

        # Use a persistent profile to maintain session
        profile_path = Path.home() / ".uc_scraper_profile"
        profile_path.mkdir(exist_ok=True)
        options.add_argument(f'--user-data-dir={profile_path}')

        # Auto-detect Chrome version instead of hardcoding
        self.driver = uc.Chrome(options=options)
        self.driver.implicitly_wait(10)

        log.info("Chrome driver initialized")

    def _random_delay(self, extra: float = 0):
        """Random delay to appear more human"""
        delay = random.uniform(MIN_DELAY, MAX_DELAY) + extra
        time.sleep(delay)

    def _wait_for_cloudflare(self, timeout: int = 30):
        """Wait for Cloudflare challenge to complete"""
        log.info("Waiting for Cloudflare check...")

        try:
            # Wait for challenge to disappear
            WebDriverWait(self.driver, timeout).until_not(
                EC.presence_of_element_located((By.ID, "challenge-running"))
            )
        except TimeoutException:
            pass

        # Wait for page to load
        try:
            WebDriverWait(self.driver, timeout).until(
                EC.presence_of_element_located((By.TAG_NAME, "body"))
            )
        except TimeoutException:
            log.warning("Page load timeout")

    def _navigate(self, url: str) -> bool:
        """Navigate to URL with Cloudflare bypass"""
        self._init_driver()

        log.info(f"Navigating to: {url}")
        self.driver.get(url)
        self._wait_for_cloudflare()
        self._random_delay()

        # Check if we got blocked
        if "Access Denied" in self.driver.page_source or "Just a moment" in self.driver.title:
            log.warning("Cloudflare challenge detected, waiting...")
            time.sleep(10)
            self._wait_for_cloudflare(timeout=60)

        return True

    def scrape_forum_page(self, page_url: str) -> List[ForumThread]:
        """Scrape a single forum page for thread listings"""
        threads = []

        if not self._navigate(page_url):
            return threads

        soup = BeautifulSoup(self.driver.page_source, 'html.parser')

        # UC uses id="thread_title_XXXXX" for thread links
        thread_elements = soup.select('a[id^="thread_title_"]')

        if not thread_elements:
            # Fallback selectors
            thread_elements = soup.select('tr.threadlistrow') or soup.select('li.threadbit')

        # Known sticky thread IDs for anti-cheat-bypass forum (skip these)
        # These are pinned informational threads, not useful for scraping
        STICKY_THREAD_IDS = {
            '333662',  # All methods of retrieving HWIDs
            '75304',   # Anti-cheat index
            '161321',  # Complete Sources & Releases List
            '728387',  # Anti-Cheat Bypass Rules
        }

        for elem in thread_elements:
            # Skip sticky threads by known IDs
            elem_id = elem.get('id', '')
            thread_num = elem_id.replace('thread_title_', '') if elem_id else ''
            if thread_num in STICKY_THREAD_IDS:
                log.debug(f"Skipping sticky: {elem.get_text(strip=True)[:50]}")
                continue
            try:
                thread = self._parse_thread_element(elem, page_url)
                if thread:
                    threads.append(thread)
                    self.db.save_thread(thread)
            except Exception as e:
                log.error(f"Error parsing thread: {e}")
                continue

        log.info(f"Found {len(threads)} threads on page")
        return threads

    def _parse_thread_element(self, elem, page_url: str) -> Optional[ForumThread]:
        """Parse a thread element from the forum listing"""
        try:
            # UC uses id="thread_title_XXXXX" pattern
            title_elem = elem

            title = title_elem.get_text(strip=True)
            url = title_elem.get('href', '')
            if not url:
                return None

            # Clean session ID from URL
            if '?' in url:
                url = url.split('?')[0]

            # Extract thread ID from URL
            thread_id = self._extract_thread_id(url)
            if not thread_id:
                return None

            # Find parent row to get other info
            parent = elem.find_parent('tr')

            author = "Unknown"
            replies = 0
            views = 0
            last_post = ""

            if parent:
                # Author is in td with class containing 'author' or 'starter'
                author_elem = parent.select_one('td.alt2 a.username, a.username')
                if author_elem:
                    author = author_elem.get_text(strip=True)

                # Get all td cells
                cells = parent.select('td')
                for cell in cells:
                    text = cell.get_text(strip=True)
                    # Look for reply/view counts (usually just numbers)
                    if text.isdigit() or text.replace(',', '').isdigit():
                        num = self._parse_number(text)
                        if replies == 0:
                            replies = num
                        elif views == 0:
                            views = num

                # Last post date
                date_elem = parent.select_one('td.alt1 .time, span.time')
                if date_elem:
                    last_post = date_elem.get_text(strip=True)

            # Subforum (from page URL or breadcrumb)
            subforum = urlparse(page_url).path.split('/')[-2] if '/' in page_url else "unknown"

            return ForumThread(
                thread_id=thread_id,
                title=title,
                url=url,
                author=author,
                replies=replies,
                views=views,
                last_post_date=last_post,
                subforum=subforum,
                scraped_at=datetime.now().isoformat()
            )

        except Exception as e:
            log.debug(f"Thread parse error: {e}")
            return None

    def _extract_thread_id(self, url: str) -> Optional[str]:
        """Extract thread ID from URL"""
        # Pattern: showthread.php?t=123456 or /forum/thread-123456.html
        import re

        patterns = [
            r't=(\d+)',
            r'thread-(\d+)',
            r'/(\d+)-',
            r'-(\d+)\.html'
        ]

        for pattern in patterns:
            match = re.search(pattern, url)
            if match:
                return match.group(1)

        return None

    def _parse_number(self, text: str) -> int:
        """Parse numbers from text like '1,234 views' or '5.6K'"""
        import re

        text = text.lower().replace(',', '').replace(' ', '')

        # Handle K/M suffixes
        if 'k' in text:
            match = re.search(r'([\d.]+)k', text)
            if match:
                return int(float(match.group(1)) * 1000)
        elif 'm' in text:
            match = re.search(r'([\d.]+)m', text)
            if match:
                return int(float(match.group(1)) * 1000000)

        # Plain number
        match = re.search(r'(\d+)', text)
        return int(match.group(1)) if match else 0

    def scrape_thread_content(self, thread_url: str, thread_id: str) -> List[ForumPost]:
        """Scrape all posts from a thread"""
        posts = []

        if not self._navigate(thread_url):
            return posts

        # Get all pages of the thread
        page_urls = [thread_url]

        # Find pagination
        soup = BeautifulSoup(self.driver.page_source, 'html.parser')
        pagination = soup.select('div.pagination a') or soup.select('span.pagenav a')

        for link in pagination:
            href = link.get('href', '')
            if href and 'page=' in href or '-page' in href:
                full_url = urljoin(BASE_URL, href)
                if full_url not in page_urls:
                    page_urls.append(full_url)

        # Limit pages to avoid excessive scraping
        page_urls = page_urls[:10]

        for i, page_url in enumerate(page_urls):
            if i > 0:  # Already on first page
                self._navigate(page_url)

            page_posts = self._parse_posts(thread_id)
            posts.extend(page_posts)

            for post in page_posts:
                self.db.save_post(post)

            log.info(f"Scraped {len(page_posts)} posts from page {i+1}/{len(page_urls)}")

            if i < len(page_urls) - 1:
                self._random_delay(PAGE_DELAY)

        self.db.mark_thread_scraped(thread_id)
        return posts

    def _parse_posts(self, thread_id: str) -> List[ForumPost]:
        """Parse all posts from current page"""
        posts = []
        soup = BeautifulSoup(self.driver.page_source, 'html.parser')

        post_elements = (
            soup.select('div.postbit') or
            soup.select('li.postbit') or
            soup.select('div.post') or
            soup.select('table.post')
        )

        for i, elem in enumerate(post_elements):
            try:
                post = self._parse_post_element(elem, thread_id, is_op=(i == 0))
                if post:
                    posts.append(post)
            except Exception as e:
                log.debug(f"Post parse error: {e}")
                continue

        return posts

    def _parse_post_element(self, elem, thread_id: str, is_op: bool = False) -> Optional[ForumPost]:
        """Parse a single post element"""
        try:
            # Post ID
            post_id = elem.get('id', '') or elem.get('data-postid', '')
            if not post_id:
                post_id = hashlib.md5(elem.get_text()[:100].encode()).hexdigest()[:16]

            # Author
            author_elem = (
                elem.select_one('a.username') or
                elem.select_one('span.username') or
                elem.select_one('div.username a')
            )
            author = author_elem.get_text(strip=True) if author_elem else "Unknown"

            # Reputation
            rep_elem = elem.select_one('span.reputation') or elem.select_one('div.reputation')
            reputation = rep_elem.get_text(strip=True) if rep_elem else None

            # Content
            content_elem = (
                elem.select_one('div.postcontent') or
                elem.select_one('td.postcontent') or
                elem.select_one('div.content') or
                elem.select_one('blockquote.postcontent')
            )

            if not content_elem:
                return None

            # Extract code blocks before stripping
            code_blocks = []
            for code_elem in content_elem.select('pre, code, div.code, div.php'):
                code_blocks.append(code_elem.get_text(strip=True))

            content = content_elem.get_text(strip=True)

            # Attachments
            attachments = []
            for attach in elem.select('a.attachment') or elem.select('div.attachments a'):
                attachments.append(attach.get('href', ''))

            # Post date
            date_elem = (
                elem.select_one('span.date') or
                elem.select_one('span.postdate') or
                elem.select_one('div.postdate')
            )
            post_date = date_elem.get_text(strip=True) if date_elem else ""

            return ForumPost(
                post_id=str(post_id),
                thread_id=thread_id,
                author=author,
                author_reputation=reputation,
                content=content,
                post_date=post_date,
                is_op=is_op,
                code_blocks=code_blocks,
                attachments=attachments,
                scraped_at=datetime.now().isoformat()
            )

        except Exception as e:
            log.debug(f"Post element parse error: {e}")
            return None

    def scrape_forum(self, max_pages: int = 10, scrape_content: bool = True):
        """Main scraping loop for the anti-cheat bypass forum"""
        log.info(f"Starting forum scrape (max {max_pages} pages)")

        all_threads = []

        # Scrape thread listings
        for page_num in range(1, max_pages + 1):
            if page_num == 1:
                page_url = FORUM_URL
            else:
                page_url = f"{FORUM_URL}index{page_num}.html"

            log.info(f"Scraping page {page_num}/{max_pages}...")

            try:
                threads = self.scrape_forum_page(page_url)
                all_threads.extend(threads)

                if not threads:
                    log.info("No more threads found, stopping pagination")
                    break

                self._random_delay(PAGE_DELAY)

            except Exception as e:
                log.error(f"Error on page {page_num}: {e}")
                continue

        log.info(f"Thread listing complete: {len(all_threads)} threads found")

        # Optionally scrape thread contents
        if scrape_content:
            unscraped = self.db.get_unscraped_threads(limit=50)
            log.info(f"Scraping content for {len(unscraped)} threads...")

            for i, thread_info in enumerate(unscraped):
                try:
                    log.info(f"[{i+1}/{len(unscraped)}] Scraping: {thread_info['title'][:50]}...")
                    self.scrape_thread_content(thread_info['url'], thread_info['thread_id'])
                    self._random_delay(PAGE_DELAY)

                except Exception as e:
                    log.error(f"Error scraping thread: {e}")
                    continue

        stats = self.db.get_stats()
        log.info(f"Scraping complete! Stats: {stats}")

        return stats

    def search_intel(self, query: str) -> List[dict]:
        """Search scraped content"""
        return self.db.search(query)

    def export_json(self, output_path: str = "uc_intel_export.json"):
        """Export all data to JSON"""
        cursor = self.db.conn.cursor()

        cursor.execute("SELECT * FROM threads ORDER BY views DESC")
        threads = [{
            "thread_id": r[0],
            "title": r[1],
            "url": r[2],
            "author": r[3],
            "replies": r[4],
            "views": r[5],
            "last_post_date": r[6],
            "subforum": r[7]
        } for r in cursor.fetchall()]

        cursor.execute("SELECT * FROM posts")
        posts = [{
            "post_id": r[0],
            "thread_id": r[1],
            "author": r[2],
            "content": r[4],
            "code_blocks": json.loads(r[7]) if r[7] else []
        } for r in cursor.fetchall()]

        export_data = {
            "exported_at": datetime.now().isoformat(),
            "stats": self.db.get_stats(),
            "threads": threads,
            "posts": posts
        }

        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(export_data, f, indent=2, ensure_ascii=False)

        log.info(f"Exported to {output_path}")
        return output_path

    def close(self):
        """Cleanup"""
        if self.driver:
            self.driver.quit()
        self.db.close()


def main():
    """CLI interface"""
    import argparse

    parser = argparse.ArgumentParser(description="UnknownCheats Forum Scraper")
    parser.add_argument("--pages", type=int, default=5, help="Number of forum pages to scrape")
    parser.add_argument("--no-content", action="store_true", help="Skip scraping thread contents")
    parser.add_argument("--search", type=str, help="Search scraped content")
    parser.add_argument("--export", action="store_true", help="Export to JSON")
    parser.add_argument("--headless", action="store_true", default=True, help="Run headless (default)")
    parser.add_argument("--visible", action="store_true", help="Show browser window")
    parser.add_argument("--stats", action="store_true", help="Show database stats")
    parser.add_argument("--db", type=str, default="uc_intel.db", help="Database path")

    args = parser.parse_args()

    scraper = UCScraper(headless=not args.visible, db_path=args.db)

    try:
        if args.stats:
            stats = scraper.db.get_stats()
            print(f"\nDatabase Statistics:")
            print(f"  Threads:        {stats['threads']}")
            print(f"  Posts:          {stats['posts']}")
            print(f"  With content:   {stats['threads_with_content']}")

        elif args.search:
            results = scraper.search_intel(args.search)
            print(f"\nSearch results for '{args.search}':")
            for r in results[:10]:
                print(f"\n[{r['thread_title'][:50]}] by {r['author']}")
                print(f"  {r['content'][:200]}...")
                print(f"  URL: {r['url']}")

        elif args.export:
            scraper.export_json()

        else:
            scraper.scrape_forum(
                max_pages=args.pages,
                scrape_content=not args.no_content
            )

    finally:
        scraper.close()


if __name__ == "__main__":
    main()
