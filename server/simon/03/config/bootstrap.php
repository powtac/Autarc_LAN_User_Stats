<?php

require_once dirname(__FILE__).'/../misc/basics.php';

db_check();

$DB = db_init();
$DB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

if (strlen($_SERVER['REQUEST_METHOD'])) {
	define('HTTP_TYPE', $_SERVER['REQUEST_METHOD']);
}

if (!defined('HTTP_TYPE')) {
	define('HTTP_TYPE', NULL);
}

switch(HTTP_TYPE) {
	case 'POST':
		define('IS_POST', TRUE);
		define('IS_GET',  FALSE);
	break;
	case 'GET':
		define('IS_POST', FALSE);
		define('IS_GET',  TRUE);
	break;
	default:
		define('IS_POST', FALSE);
		define('IS_GET',  FALSE);
}


if (!empty($_SERVER['PATH_INFO'])) {
	define('URI', $_SERVER['PATH_INFO']);
} else if (strlen($_SERVER['REQUEST_URI'])) {
	define('URI', $_SERVER['REQUEST_URI']);
}

if (!defined('URI')) {
	define('URI', NULL);
}


define('BODY', trim(file_get_contents('php://input')));


define('IS_JSON', (bool)json_decode(BODY));

if (IS_JSON) {
	$JSON = json_decode(BODY);
} else {
	$JSON = FALSE;
}


// Get network_name
if (IS_JSON AND IS_POST) {
	if (!empty($JSON->network_name)) {
		define('NETWORK_NAME', $JSON->network_name);
	}
} else if(preg_match('~/network/~', URI)) {
	preg_match('~/network/([^/]+)[/]{0,1}~', URI, $matches);
	if (!empty($matches[1])) {
		define('NETWORK_NAME', $matches[1]);
	}
	unset($matches);
}


if (LOG_ALL) {
	try {
		if (isset($_SERVER['HTTP_USER_AGENT'])) {
			$ua = $_SERVER['HTTP_USER_AGENT'];
		} else {
			$ua = 'unknown';
		}
		
		file_put_contents(AUTARC_TMP.'requests.txt', 
			"-------------------\n".
			date('d.m.Y H:i:s').' '.HTTP_TYPE.' '.URI."\n".
			'User-Agent: '.@$ua."\n".
			'Params: '.var_export($_REQUEST, 1)."\n".
			'Body: '.BODY."\n\n",
			FILE_APPEND);
	} catch (Exception $e) {
		var_dump($e);
	}
}


