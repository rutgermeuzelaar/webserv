import os
import pathlib

def create_html_button(href: str, text: str) -> None:
    print(
        f"<button onclick=\"deleteRequest('/{href}')\">{text}</button"
        )
print("Content-Type: text/html", end="\r\n")
PATH_TRANSLATED = os.environ.get("PATH_TRANSLATED")

if PATH_TRANSLATED == None:
    print("Please supply a PATH_INFO environment variable")
    exit(1)

if PATH_TRANSLATED == './root':
    exit(1)

cwd = pathlib.Path.cwd()
dir = cwd.joinpath(PATH_TRANSLATED)

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