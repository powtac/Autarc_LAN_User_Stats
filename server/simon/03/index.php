<?php
header('Access-Control-Allow-Origin: *');
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");

require_once dirname(__FILE__).'/config/config.php';
require_once dirname(__FILE__).'/config/bootstrap.php';

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


// GET /network/[network_name]/device/[mac]/info
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
		$result['vendor'] 				= @file_get_contents('http://api.macvendors.com/' . urlencode($mac));
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


// GET /stats/network/[network_name][/range]
if (IS_GET AND preg_match('~/stats/network/[^/]+~', URI)) {
	preg_match('~/stats/network/[^/]+/(.+)~', URI, $matches);
	$range = !empty($matches[1]) ? urldecode(trim($matches[1])) : RANGE_DEFAULT;
	
	if (!is_int(strtotime('-'.$range))) {
		die('The range parameter "'.$range.'" has no valid value.');
	}
	
	$result['network_name'] = NETWORK_NAME;
	$result['range'] 		= $range;
	
	
	$query = $DB->prepare('SELECT * FROM pings WHERE network_name = ? AND datetime > FROM_UNIXTIME(?) ORDER BY datetime DESC');
	
	$query->execute(array(NETWORK_NAME, strtotime('-'.$range)));

	$pings = array();
	$i = 0;
	while ($data = $query->fetch()) {
		$pings[$i]['t'] 				= time() - strtotime($data['datetime']);
		$pings[$i]['d'] 				= $data['datetime'];
		$pings[$i]['ip'] 				= $data['ip'];
		$pings[$i]['status'] 			= $data['status'];
		if (strlen($data['mac'])) {
			$pings[$i]['mac'] 			= $data['mac'];
			$pings[$i]['mac_formated']	= mac_format($data['mac']);
		}
		$i++;
	}
	
	$result['data'] = $pings;

	echo return_json($result);
	exit;
}


// POST /device/name
if (IS_POST AND IS_JSON AND preg_match('~/device/name~', URI)) {
	
	// Does device name exist?
	$query = $DB->prepare('SELECT name FROM devices_neu WHERE network_name = ? AND mac = ?');
	$query->execute(array(NETWORK_NAME, $JSON->mac));
	
	// UPDATE / INSERT
	// TODO check REPLACE INTO syntax
	if(!$query->fetch()) {
		$query = $DB->prepare('INSERT INTO devices_neu (`network_name`, `mac`, `name`) VALUES (?, ?, ?)');
		$query->execute(array(NETWORK_NAME, $JSON->mac, $JSON->name));
	} else {
		$query = $DB->prepare('UPDATE devices_neu SET name = ? WHERE network_name = ? AND mac = ?');
		$query->execute(array($JSON->name, NETWORK_NAME, $JSON->mac));
	}
	echo 'OK'; // TODO verify
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
	#header('501 Not Implemented', true, 501);
}


// TODO display proper documentation
$apis[] = 'GET /debug/phpinfo';
$apis[] = 'GET /debug/requests';
$apis[] = 'GET /network/[network_name]/device/[mac]/info';
$apis[] = 'GET /networks/list';
$apis[] = 'GET /stats/network/[network_name][/range]';
$apis[] = 'POST /device/name';
$apis[] = 'POST /ping_result/add';
sort($apis);

echo '{"message": "'.HTTP_TYPE.' '.URI.' not Implemented", "available_apis":';
echo return_json($apis);
echo '}';

