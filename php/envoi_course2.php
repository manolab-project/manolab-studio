<?php
  /*
   Ce fichier a la tâche d'envoyer les données de la course au logiciel Manolab.
   Il est appelé par Manolab (ou toute autre entité externe, le client.
   
    1. Récupérer toutes les données de la course en cours  avec une requête MySQL
    2. Transformer cela en JSON
    3. Répoondre au client (Manolab) avec ce fichier
  */
 

$mysqli = new mysqli("mysql660.sql004:3306;", "mdbnfrjgrg323", "mdbnfrjgrg323", "BjvQg3Qbrrxg");
$myArray = array();

if ($result = $mysqli->query("SELECT * FROM mod50_visforms_1")) {
    header('Content-type: application/json');
    while($row = $result->fetch_array(MYSQLI_ASSOC)) {
            $myArray[] = $row;
    }
    echo json_encode($myArray);
}
echo 'ok';
$result->close();
$mysqli->close();
 
?>