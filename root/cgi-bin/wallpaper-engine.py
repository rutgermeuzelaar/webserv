import sys
import urllib.parse
import os

LINE_BREAK = "\r\n"
CONTENT_TYPE = os.environ.get("CONTENT_TYPE")
CONTENT_LENGTH = os.environ.get("CONTENT_LENGTH")
REQUEST_METHOD = os.environ.get("REQUEST_METHOD")

if REQUEST_METHOD != "POST":
    print("Status: 405", end=LINE_BREAK)
    print("Allow: POST", end=LINE_BREAK)
    print("Content-Type: text/plain", end=LINE_BREAK)
    print(end=LINE_BREAK)
    print("405 - Method Not Allowed")
    exit(0)

assert CONTENT_LENGTH != None, "CONTENT_LENGTH environment variable cannot be None"
lines = sys.stdin.read(int(CONTENT_LENGTH))
parsed_qs = urllib.parse.parse_qs(lines)
print("Content-Type: text/html", end="\r\n")
print(end=LINE_BREAK)
print(
    "<html>"
    "<head>"
        "<style>"
        "body {"
        f"   background-color: {parsed_qs.get('favorite-color')[0]};"
        "}"
        "</style>"
    "</head>"
    "<body>"
    "</body>"
    "</html>"
)
