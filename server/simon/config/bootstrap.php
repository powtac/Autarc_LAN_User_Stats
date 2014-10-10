<?php

require_once dirname(__FILE__).'/../libs/klein.php';

// Update request when we have a subdirectory
// https://github.com/chriso/klein.php/wiki/Sub-Directory-Installation
if(ltrim(dirname($_SERVER['PHP_SELF']), '/')){ 
	$_SERVER['REQUEST_URI'] = substr($_SERVER['REQUEST_URI'], strlen(dirname($_SERVER['PHP_SELF'])));
}
