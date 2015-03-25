<?php

function mac_format($mac, $separator = ':', $uppercase = TRUE) {
    if (is_array($mac)) {
        $mac = implode($separator, $mac);
    } else {
        $mac = str_replace(array(':', '.', '-', '_', ' ', ','), $separator, $mac);
    }
    
    if ($uppercase) {
        $mac = ucfirst($mac);
    }
    
    return $mac;
}

function error($msg) {
    echo '<div style="border:1px solid red; color: red; margin: 5px; padding-left: 10px; border-radius: 3px">'.$msg.'</div>';
}


function db_check() {
    try {
        $dbh = new PDO(DB_DNS, DB_USER, DB_PASS);
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

function ip_formt($ip) {
	$ip = trim($ip);
	if (!filter_var($ip, FILTER_VALIDATE_IP) === false) {
		// ok
	} else {
		echo 'IP "'.$ip.'" malformed';
	}
	return $ip;
}

