# Import modules for CGI handling
import cgi, cgitb
# Create instance of FieldStorage
form = cgi.FieldStorage()
# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')
print("Hello %s %s" % (first_name, last_name))
