<?php
$imageUrl = '1337.png';

$imageData = file_get_contents($imageUrl);

if ($imageData !== false) {
    // Set the content type as image/jpeg
    header('Content-type: image/png');

    echo $imageData;
} else {
    echo 'Failed to retrieve the image.';
}
?>