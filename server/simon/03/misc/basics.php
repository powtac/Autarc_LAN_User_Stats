<?php

function mac_format($mac, $separator = ':', $uppercase = TRUE) {
    if (is_array($mac)) {
        $mac = implode($separator, $mac);
    } else {
        $mac = str_replace(array(':', '.', '-', '_', ' ', ','), $separator, $mac);
    }
    
    if ($uppercase) {
        $mac = strtoupper($mac);
	} else {
		$mac = strtolower($mac);
	}
    
    return $mac;
}


function error($msg) {
    echo '<div style="border:1px solid red; color: red; margin: 5px; padding-left: 10px; border-radius: 3px">'.$msg.'</div>';
}


function db_check() {
    try {
        $DB = new PDO(DB_DNS, DB_USER, DB_PASS);
		$DB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    } catch (PDOException $e) {
        error('Connection failed: ' . $e->getMessage());
    }
}


function db_init($reset = FALSE) {
    global $DB;
    
    if ($reset OR !($DB instanceof PDO)) {
        return new PDO(DB_DNS, DB_USER, DB_PASS, array(PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES \'UTF8\''));
    }
    
    return $DB;
}


function return_json($data = NULL) {
	if (!headers_sent()) {
		header('Content-Type: application/json');
	}
	
	// TODO refactor
	if (json_encode(@json_decode($data)) == $data) {
		return $data;
	} else {
		return json_encode($data);
	}
}
