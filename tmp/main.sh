export "CONTENT_LENGTH=100"
# export "CONTENT_TYPE=text/html",
export "REQUEST_METHOD=POST"
# export "PATH_INFO=url/test.php"
# export "SERVER_PROTOCOL=HTTP/1.1",
# export "SCRIPT_NAME=cgi-bin/php-cgi",
# export "SERVER_PORT=8080",
# export "HTTP_HOST=0.0.0.0",
# export "GATEWAY_INTERFACE=CGI/1.1",
# export "REQUEST_url=test.php",
export "REDIRECT_STATUS=200"
# export "QUERY_STRING="
# export "HTTP_COOKIE="
export "SCRIPT_FILENAME=/Users/kadjane/Desktop/server_1 copy/tmp/test.php"
# SCRIPT_NAME="/cgi-bin/printenv.pl"


./php-cgi ./test.php < in_file 