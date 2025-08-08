import os
import pathlib

LINE_BREAK = "\r\n"
REQUEST_METHOD = os.environ.get("REQUEST_METHOD")
PATH_TRANSLATED = os.environ.get("PATH_TRANSLATED")

def create_html_button(href: str, text: str) -> None:
    print(
        f"<button onclick=\"deleteRequest('/{href}')\">{text}</button"
        )

def create_response_page(status_code: int, status_message: str) -> None:
    print(f"Status: {status_code}", end=LINE_BREAK)
    print("Content-Type: text/plain", end=LINE_BREAK)
    print("Status: 403", end=LINE_BREAK)
    print(end=LINE_BREAK)
    print(f"{status_code} - {status_message}")

if REQUEST_METHOD != "GET":
    print("Allow: GET", end=LINE_BREAK)
    create_response_page(405, "Method Not Allowed")
    exit(0)

if PATH_TRANSLATED == None:
    create_response_page(400, "Bad Request")
    print("Please supply a PATH_INFO environment variable")
    exit(0)

if PATH_TRANSLATED == './root':
    create_response_page(403, "Forbidden")
    exit(0)

cwd = pathlib.Path.cwd()
dir = cwd.joinpath(PATH_TRANSLATED)

print("Content-Type: text/html", end="\r\n")
print(end="\r\n")
print(
    "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
        "<head>"
		    "<meta charset=\"utf-8\"/>"
            "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
		    f"<title>{ pathlib.Path(__file__).resolve().name }</title>"
        )

print("<script src=\"/root/scripts/delete-request.js\"></script>")
print("</head>")
print(f"<h1>{ os.environ.get('PATH_INFO')}</h1>")
print("<hr>")
print(
    "<table>" \
        "<tr>" \
            "<th>name</th>"
            "<th></th>"
        "</tr>"
)

for path in dir.iterdir():
    rel_path = path.relative_to(cwd)
    print(f"<tr><td>{rel_path}</td>")
    print("<td>", end=None)
    create_html_button(rel_path, "Delete")
    print("</td></tr>")
print("</table>")
print("<hr>")
print("<a href=\"/\">Home</a>")
print("</html>")
exit(0)
