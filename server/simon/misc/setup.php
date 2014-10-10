<?php

// Bootstrap
require_once dirname(__FILE__).'/config.php';
require_once dirname(__FILE__).'/basics.php';


// Create DB
//if (!file_exists(DB_PATH)) {
//    echo 'Creating DB...<br />';
//    
//    if (!is_writable(dirname(DB_PATH))) {
//        error('Can not create Database at '.dirname(DB_PATH).', no folder permissions.');
//    } else {
//        if (file_put_contents(DB_PATH, '') === FALSE) {
//            error('Can not create Database at '.DB_PATH.', no file permissions.');
//        }
//    }
//}


// Check write permissions
//if (!is_writable(DB_PATH)) {
//    error('Can not write into Database at '.DB_PATH.', no permissions.');
//}


// Intialize DB Schema
