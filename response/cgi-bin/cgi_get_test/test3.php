<!DOCTYPE html>
<html>
<head>
    <title>Name and Age</title>
</head>
<body>
    <form action="" method="GET">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name" required><br><br>

        <label for="age">Age:</label>
        <input type="number" name="age" id="age" required><br><br>

        <input type="submit" value="Submit">
    </form>

    <?php
    if (isset($_GET['name']) && isset($_GET['age'])) {
        // Retrieve the submitted name and age
        $name = $_GET['name'];
        $age = $_GET['age'];

        // Display the output
        echo '<p style="color: green;">The age of ' . $name .  ' is: ' . $age . '</p>';
    }
    ?>
</body>
</html>





<!-- X-Powered-By: PHP/8.2.1
Content-type: text/html; charset=UTF-8

<!DOCTYPE html>
<html>
<head>
    <title>Name and Age</title>
</head>
<body>
    <form action="" method="GET">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name" required><br><br>

        <label for="age">Age:</label>
        <input type="number" name="age" id="age" required><br><br>

        <input type="submit" value="Submit">
    </form>

    <p style="color: green;">The age of jklvxjv is: 5424</p></body>
</html> -->