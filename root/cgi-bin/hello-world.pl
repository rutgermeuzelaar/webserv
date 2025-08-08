if ($ENV{REQUEST_METHOD} ne "GET")
{
    print "Status: 405\r\n";
    print "Allow: GET\r\n";
    print "Content-Type: text/plain\r\n\r\n";
    print "405 - Method Not Allowed";
    exit 0;
}

print "Content-Type: text/html\r\n";
print "\r\n";
print "Hello world!\n";
