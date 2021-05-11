<?php
  /*
   Ce fichier a la tâche d'envoyer les données de la course au logiciel Manolab.
   Il est appelé par Manolab (ou toute autre entité externe, le client.
    
    1. Récupérer toutes les données de la course en cours  avec une requête MySQL
    2. Transformer cela en JSON
    3. Répoondre au client (Manolab) avec ce fichier
  */
  
//                    adress        user       pass     db  
// $mysqli = new mysqli("127.0.0.1", "anthony", "1234", "pierobois");
$mysqli = new mysqli("mdbnsig168.mysql.db:3306", "mdbnsig168", "cKfnGnxx7jdy", "mdbnsig168");

// Oh no! A connect_errno exists so the connection attempt failed!
if ($mysqli->connect_errno) {
    // The connection failed. What do you want to do? 
    // You could contact yourself (email?), log the error, show a nice page, etc.
    // You do not want to reveal sensitive information

    // Let's try this:
    echo "Sorry, this website is experiencing problems.";

    // Something you should not do on a public site, but this example will show you
    // anyways, is print out MySQL error related information -- you might log this
    echo "Error: Failed to make a MySQL connection, here is why: \n";
    echo "Errno: " . $mysqli->connect_errno . "\n";
    echo "Error: " . $mysqli->connect_error . "\n";
    
    // You might want to show them something nice, but we will simply exit
    exit;
}

// echo 'PHP version: ' . phpversion();


if ($result = $mysqli->query("SELECT * FROM manolab_results")) {

  //  echo "Returned rows are: " . $result -> num_rows;
    echo "ID     TOURS       TEMPS  <br /> ";
    
    while($row = $result->fetch_array(MYSQLI_ASSOC)) {
         echo "{$row['id']}     {$row['tours']}     {$row['temps']}<br />--------------------------------<br />";
    }

}
else 
{
    echo 'problem: '.$result;
}

$result->close();
$mysqli->close();
 
?>
