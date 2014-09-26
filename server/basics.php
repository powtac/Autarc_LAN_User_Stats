<?php

function mac_format($mac, $separator = ':', $uppercase = TRUE) {
    if (is_array($mac)) {
        $mac = implode($separator, $mac);
    } else {
        $mac = str_replace(array(':', '.', '-', '_', ' ', ','), $separator, $mac);
    }

    if ($uppercase) {
        $mac = strtoupper($mac);
    }

    return $mac;
}
?>