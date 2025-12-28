#!/usr/bin/env python3
"""Debug script to capture and analyze UC forum HTML structure"""

import undetected_chromedriver as uc
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from bs4 import BeautifulSoup
import time

URL = "https://www.unknowncheats.me/forum/anti-cheat-bypass/"

print("[*] Starting Chrome...")
options = uc.ChromeOptions()
options.add_argument('--no-sandbox')
options.add_argument('--window-size=1920,1080')

driver = uc.Chrome(options=options)
driver.implicitly_wait(10)

print(f"[*] Navigating to {URL}")
driver.get(URL)

print("[*] Waiting for Cloudflare...")
time.sleep(10)

print(f"[*] Page title: {driver.title}")
print(f"[*] Current URL: {driver.current_url}")

# Save page source for analysis
html = driver.page_source
with open("debug_page.html", "w", encoding="utf-8") as f:
    f.write(html)
print("[+] Saved page source to debug_page.html")

# Analyze structure
soup = BeautifulSoup(html, 'html.parser')

print("\n[*] Looking for thread elements...")

# Try various selectors
selectors = [
    'tr.threadlistrow',
    'li.threadbit',
    'div.thread_entry',
    'a.thread-title',
    'td.threadtitle',
    'div[id^="thread_"]',
    'li[id^="thread_"]',
    'tr[id^="thread_"]',
    'a[href*="showthread"]',
    'h3.threadtitle',
]

for sel in selectors:
    found = soup.select(sel)
    if found:
        print(f"  [+] '{sel}': Found {len(found)} elements")
        if len(found) > 0:
            print(f"      First: {str(found[0])[:200]}...")
    else:
        print(f"  [-] '{sel}': Not found")

# Check if we're on a login page or something
if "login" in html.lower():
    print("\n[!] Login form detected - may need to authenticate")

if "cloudflare" in html.lower():
    print("\n[!] Cloudflare content still present - challenge may not be complete")

print("\n[*] First 500 chars of body:")
body = soup.find('body')
if body:
    print(body.get_text()[:500])

driver.quit()
print("\n[+] Done")
