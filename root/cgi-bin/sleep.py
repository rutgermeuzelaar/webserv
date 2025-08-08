import time
import os

LINE_BREAK = "\r\n"
REQUEST_METHOD = os.environ.get("REQUEST_METHOD")

if REQUEST_METHOD != "GET":
    print("Status: 405", end=LINE_BREAK)
    print("Allow: GET", end=LINE_BREAK)
    print("Content-Type: text/plain", end=LINE_BREAK)
    print(end=LINE_BREAK)
    print("405 - Method Not Allowed")
    exit(0)

print("Content-Type: text/html", end="\r\n")
print(end="\r\n")
time.sleep(11)
