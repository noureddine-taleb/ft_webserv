<?php
// Get the value of the "name" parameter from the query string
$name = $_GET['name'] ?? 'Anonymous';

// Generate the response
$responseBody = "Hello, $name! This is a GET request.";

// Set the content-type header
// header("Content-Type: text/html");

// Print the response body
echo $responseBody;
?>