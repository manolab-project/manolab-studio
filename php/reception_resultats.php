<?php
    $body = file_get_contents('php://input');

    // debug
    file_put_contents("php://stdout", "$body\r\n");
 
    //create a DB connection
    
//     $mysqli = new mysqli("127.0.0.1", "anthony", "1234", "pierobois");
    $mysqli = new mysqli("mdbnsig168.mysql.db:3306", "mdbnsig168", "cKfnGnxx7jdy", "mdbnsig168");
    $mysqli->query("DROP TABLE manolab_results");
    $mysqli->query("CREATE TABLE IF NOT EXISTS manolab_results (
                id BIGINT,
                tours LONGTEXT NOT NULL,
                temps LONGTEXT NOT NULL)");

    $result = json_decode($body);
    

    foreach($result as $key => $value) {
        if($value) {
            file_put_contents("php://stdout", "Val: $value->id\r\n");
            // how to use json array to insert data in Database
            $sql_res = $mysqli->query("INSERT INTO manolab_results (id, tours, temps) VALUES ($value->id, '" . $value->tours . "','" . $value->temps . "')");
            
            if (!$sql_res) {
                file_put_contents("php://stdout",  "Echec lors de la création de la table : (" . $mysqli->errno . ") " . $mysqli->error . "\r\n");
            }
        }
        else {
            file_put_contents("php://stdout", "Err: $value\r\n");
        }
    }
    
    $mysqli->close();

    // On répond ok ou error
    echo 'ok';
?>
