import requests
from bs4 import BeautifulSoup
import re

url = "https://hamaward.cloud/wwa/teams"

headers = {
    "User-Agent": "Mozilla/5.0"
}

html = requests.get(url, headers=headers, timeout=20).text
soup = BeautifulSoup(html, "html.parser")

text = soup.get_text("\n")
lines = [line.strip() for line in text.splitlines() if line.strip()]

pattern = re.compile(r"^[A-Z0-9/]+WWA$")

calls = []

for line in lines:
    if pattern.fullmatch(line):
        calls.append(line)

# Remove duplicates while preserving order
calls = list(dict.fromkeys(calls))

# Print in quoted, comma-separated format
for call in calls:
    print(f'"{call}",')

print(f"\nFound {len(calls)} activator calls.")