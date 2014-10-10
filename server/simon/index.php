<?php

require_once dirname(__FILE__).'/config/config.php';
require_once dirname(__FILE__).'/config/bootstrap.php';

respond(function () {
    echo '<br />Index<br />';
});

respond('/[:name]', function ($request) {
    echo 'Hello ' . $request->name;
});



with('/api/store/mac', AUTARC_ROOT_DIR.'controller/record_store.php');

respond('GET', '/', function($request, $response) {
	$response->dump($request);
});

dispatch();
