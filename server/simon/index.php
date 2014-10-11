<?php

require_once dirname(__FILE__).'/config/config.php';
require_once dirname(__FILE__).'/config/bootstrap.php';


//// Tests
//respond(function () {
//    echo '<br />Index<br />';
//});
//
//respond('/[:name]', function ($request) {
//    echo 'Hello ' . $request->name;
//});
//
//respond('GET', '/', function($request, $response) {
//	$response->dump($request);
//});
//// Tests end

//respond('POST', '*', function ($request, $response) {
//	$response->dump($request->params());
//});


// See https://github.com/powtac/Autarc_LAN_User_Stats/issues/10#issuecomment-55624432
with('/api/store/mac', AUTARC_CONTROLLER.'store_mac.php');


dispatch();
