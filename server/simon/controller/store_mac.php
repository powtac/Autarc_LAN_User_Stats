<?php
require_once dirname(__FILE__).'/../config/config.php';
require_once dirname(__FILE__).'/../config/bootstrap.php';

// store mac address
respond('POST', '[*]', function ($request, $response) {
	
	
	$data = json_decode($request->body());
	#$headers = $request->header();
	
	file_put_contents(AUTARC_TMP.'requests.txt', 
		"-------------------\n".
		date('d.m.Y h:i:s').' '.$request->method().' '.$request->uri()."\n".
		'User-Agent: '.$request->userAgent()."\n".
		'Params: '.var_export($request->params(), 1)."\n".
		'Body: '.$request->body()."\n\n",
	FILE_APPEND);
	
	$response->render(AUTARC_VIEWS.'dummy.php', array('data' => $data));
});