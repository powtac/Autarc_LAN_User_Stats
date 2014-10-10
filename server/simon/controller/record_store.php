<?php
require_once dirname(__FILE__).'/../config/config.php';
require_once dirname(__FILE__).'/../config/bootstrap.php';

// store mac address
respond('GET', '*', function ($request, $response) {
	$response->dump($request->params());
});