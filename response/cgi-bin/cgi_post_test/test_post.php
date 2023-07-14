<!DOCTYPE html>
<html>
<head>
    <title>Name and Age</title>
</head>
<body>
    <form action="process_form.php" method="POST" enctype="multipart/form-data">
        <label for="name">Name:</label>
        <input type="text" name="name" id="name" required><br><br>

        <label for="age">Age:</label>
        <input type="number" name="age" id="age" required><br><br>

        <input type="submit" value="Submit">
    </form>
</body>
</html>