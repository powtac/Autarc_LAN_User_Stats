<?php

require_once dirname(__FILE__).'/config/config.php';
require_once dirname(__FILE__).'/config/bootstrap.php';



#var_dump(URI);
#var_dump(HTTP_TYPE);
#var_dump(IS_GET);

#echo '<pre>';




// Required APIs
// https://github.com/powtac/Autarc_LAN_User_Stats/issues/10#issuecomment-55624432
/*
1. Receive Requests
2. Return vendor/manufacture for a given MAC address
3. List all scanners
4. List the stats for a single scanner
5. Receive the name for an device by MAC address 
6. Set the name for a device
*/

// POST /ping_result/add
if (IS_POST AND IS_JSON AND preg_match('~/ping_result/add~', URI)) {
	
	// DB add
	$query = $DB->prepare(SQL_PING_RESULT_ADD);
	if (!empty($JSON->online)) {
		foreach ($JSON->online as $data) {
			$query->execute(array(NETWORK_NAME, $data->ip, SQL_ONLINE, $data->mac, time()-(int)$data->t));
		}
	}
	
	if (!empty($JSON->offline)) {
		foreach ($JSON->offline as $data) {
			$query->execute(array(NETWORK_NAME, $data->ip, SQL_OFFLINE, NULL, (int)time()-$data->t));
		}
	}
	
	exit;
}


// GET /network/[network]/device/[mac]/info
if (IS_GET AND preg_match('~/network/.+/device/.+/info~', URI)) {
	preg_match('~/device/(.+)/info~', URI, $matches);
	$mac = $matches[1];
	
	$query = $DB->prepare('SELECT ip, mac, status, datetime FROM pings WHERE mac = ? AND network_name = ? ORDER BY datetime LIMIT 1');
	$query->execute(array($mac, NETWORK_NAME));
	$data1 = $query->fetch();
	$query = $DB->prepare('SELECT name FROM devices_neu WHERE mac = ? AND network_name = ?');
	$query->execute(array($mac, NETWORK_NAME));
	$data2 = $query->fetch();

	$result = array();
	$result['network_name']				= NETWORK_NAME;
	$result['mac'] 						= $mac;
	$result['mac_formated'] 			= mac_format($mac);
	if (!empty($data1)) {
		$result['status'] 				= $data1['status'];
		$result['vendor'] 				= @file_get_contents('http://api.macvendors.com/'+urlencode($mac));
		$result['last_seen_seconds'] 	= time() - strtotime($data1['datetime']);
		$result['last_seen_timestamp']	= strtotime($data1['datetime']);
		$result['last_seen_datetime'] 	= $data1['datetime'];
	}
	$result['name_set'] 				= false;
	if (!empty($data2) AND !empty($data2['name'])) {
		$result['name'] 				= $data2['name'];
		$result['name_set'] 			= true;
	}
	
	echo return_json($result);
	exit;
}


// GET networks/list
if (IS_GET AND preg_match('~/networks/list~', URI)) {
	$result = array();
    foreach ($DB->query(SQL_NETWORKS_LIST) as $row) {
		$result[] = $row['network_name'];
	}
	
	echo return_json($result);
	exit;
}

// GET /stats/[network_name][/range]

// POST /device/name
/*
{
    "network_name": "Fischergasse",
    "mac": "111:111:111:111:111",
    "name": "Simon iPhone"
}
*/
if (IS_POST AND IS_JSON AND preg_match('~/device/name~', URI)) {
	var_dump(NETWORK_NAME);
	var_dump($JSON->network_name);
	var_dump($JSON->mac);
	var_dump($JSON->name);
	exit;
}

// Debug
if (IS_GET AND preg_match('~/debug/requests~', URI)) {
	echo '<pre>'.file_get_contents(AUTARC_TMP.'requests.txt');
	exit;
}

// Phpinfo
if (IS_GET AND preg_match('~/debug/phpinfo~', URI)) {
	phpinfo();
	exit;
}


// Default
if (!headers_sent()) {
	header('501 Not Implemented', true, 501);
}
echo 'Not Implemented';
// TODO display documentation