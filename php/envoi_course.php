<?php
  /*
   Ce fichier a la tâche d'envoyer les données de la course au logiciel Manolab.
   Il est appelé par Manolab (ou toute autre entité externe, le client.
    
    1. Récupérer toutes les données de la course en cours  avec une requête MySQL
    2. Transformer cela en JSON
    3. Répoondre au client (Manolab) avec ce fichier
  */
  

$mysqli = new mysqli("127.0.0.1", "anthony", "1234", "pierobois");
$myArray = array();

if ($result = $mysqli->query("SELECT id, dossard, F5, F6, F7, F8, F10 FROM mod50_visforms_1")) {
    header('Content-type: application/json');
    while($row = $result->fetch_array(MYSQLI_ASSOC)) {
            $myArray[] = $row;
    }
    echo json_encode($myArray);
}

$result->close();
$mysqli->close();
 
?>
