<?php

define('AUTARC_ROOT_DIR', 	dirname(dirname(__FILE__)).'/');

define('AUTARC_TMP', 		AUTARC_ROOT_DIR.'tmp/');
define('AUTARC_CONTROLLER', AUTARC_ROOT_DIR.'controller/');
define('AUTARC_VIEWS', 		AUTARC_ROOT_DIR.'views/');

// DB
define('DB_NAME',   		'autarc_lan_user'); // NOT autarc_lan_user_stats!
define('DB_USER',   		'autarc_lan_user'); // NOT autarc_lan_user_stats!
define('DB_PASS',   		'');
define('DB_DNS',    		'mysql:dbname='.DB_NAME.';host=localhost;charset=UTF8');


