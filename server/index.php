<?php

require_once dirname(__FILE__).'/basics.php';
require_once dirname(__FILE__).'/mysql.php';

header('Content-Type: text/html; charset=utf-8');

$output = '<!DOCTYPE html>
<html>
    <head>
        <title>Autarc-Lan-User-Stat</title>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <link rel="icon" type="image/ico" href="http://lan-user.danit.de/favicon.ico" />
        <style type="text/css">
        <!--
        textarea{
            white-space:nowrap;
            overflow:scroll;
        }
        -->
        </style>
    </head>

    <body>';

if (isset($_GET['AVR_ID'])) {
    $AVR_ID = mysql_real_escape_string($_GET['AVR_ID']);
    $date = time();

    if (isset($_GET['MAC'])) {
    /* New online data */
        for ($i = 0; $i < count($_GET['MAC']); $i++)
        {
            $stored = false;
            $currIP = mysql_real_escape_string(htmlspecialchars($_GET['IP'][$i], ENT_QUOTES));
            $currMAC = mysql_real_escape_string(htmlspecialchars($_GET['MAC'][$i], ENT_QUOTES));

            if ($currMAC == "0:0:0:0:0:0") {
                /* Device is offline -> Update last_scan time */
                $mysql_updateScanTime = mysql_query("UPDATE Online_DB SET last_scan = '".$date."' WHERE AVR_ID='".$AVR_ID."' AND IP = '".$currIP."'");
            }
            else {
                $check_queryMACstored = mysql_query("SELECT id FROM Online_DB WHERE AVR_ID='".$AVR_ID."' AND MAC = '".$currMAC."' LIMIT 1");
                if (mysql_num_rows($check_queryMACstored) == 0) {
                /* MAC not stored yet */
                    /* new Device -> Add MAC with IP and time_offline */
                    $mysql_addMAC = mysql_query("INSERT INTO Online_DB (AVR_ID, MAC, IP, time_offline, last_scan) VALUES ('".$AVR_ID."','".$currMAC."', '".$currIP."', '".$date."', '".$date."')");
                }
                else {
                /* MAC is stored */
                    $check_queryIPstored = mysql_query("SELECT id FROM Online_DB WHERE AVR_ID='".$AVR_ID."' AND MAC = '".$currMAC."' AND IP = '".$currIP."' LIMIT 1");
                    if (mysql_num_rows($check_queryIPstored) == 0) {
                    /* IP not stored yet */
                        /* Device have a new IP -> Update IP and time_offline */
                        $mysql_updateIP = mysql_query("UPDATE Online_DB SET IP = '".$currIP."', time_offline = '".$date."', last_scan = '".$date."' WHERE AVR_ID = '".$AVR_ID."' AND MAC = '".$currMAC."' LIMIT 1");
                    }
                    else {
                    /* IP is also stored */
                        /* Device is still online -> Update offline_time */
                        $mysql_updateTime = mysql_query("UPDATE Online_DB SET time_offline = '".$date."', last_scan = '".$date."' WHERE AVR_ID='".$AVR_ID."' AND MAC = '".$currMAC."' AND IP = '".$currIP."' LIMIT 1");
                    }
                }
            }
        }
    }
    else {
    /* Print the Online-List */
	$output .= '<!--
	Last Request:
';
	/* $output .= print_r (unserialize(file_get_contents("req.txt")), true); */

	$file = file("req.txt");
	foreach($file AS $line)
	{
		$output .= print_r (unserialize($line), true);
	}

	$output .= '-->';
        $output .= 'Folgende Ger&aumlte sind online: <br /><br />';
        $output .= '<table border="0" width="50%">';
        $output .= '<tr><th>IP</th><th>MAC</th><th>Offline seit</th><th>Letzte &Uumlberpr&uumlfung: </th></tr>';
        $online_list = mysql_query("SELECT MAC, IP, time_offline, last_scan FROM Online_DB WHERE AVR_ID='".$AVR_ID."' ORDER BY IP ASC");
        while ($row = mysql_fetch_assoc($online_list))
        {
            $timediff = $date - $row['time_offline'];
            $output .= "<tr><td>".$row['IP']."</td><td><a href=\"http://www.coffer.com/mac_find/?string=".urlencode(mac_format($row['MAC'])).""\" target=\"_blank\">".mac_format($row['MAC'])."</a></td><td>".(date("j" , $timediff) - 1)." Tage ".(date("G" , $timediff) - 1)." h ".date("i" , $timediff)." min ".date("s" , $timediff)." sec</td><td>".date("Y-m-d - G:i" , $row['last_scan'])." Uhr</td></tr>";
        }
        $output .= '</table>';
    }

}
else if (isset($_GET["getAVR_ID"])) {
	$getAVR_ID = mysql_real_escape_string($_GET["getAVR_ID"]);
	if ($getAVR_ID == true) {
		echo '{"AVRID": "srvID","AVRpsw": "srvPSW"}';
	}
}
else {
    $output .= 'Bitte w&auml;hlen Sie eine AVR-ID aus:<br /><br />';
    $output .= '<form action="" method="GET" accept-charset="UTF-8"><select name='AVR_ID'>';
        $AVR_list = mysql_query("SELECT AVR_ID FROM Online_DB GROUP BY AVR_ID");
        while ($row = mysql_fetch_assoc($AVR_list))
        {
            $output .= '<option value="'.$row['AVR_ID'].'">&nbsp;'.$row['AVR_ID'].'&nbsp;</option>';
        }
    $output .= '</select><br /><br /><input type="submit" name="cmdShow" value="Anzeigen"/></form>';
}

$output .= '<a href="https://github.com/powtac/Autarc_LAN_User_Stats/"><img style="position: absolute; top: 0; right: 0; border: 0;" src="https://camo.githubusercontent.com/a6677b08c955af8400f44c6298f40e7d19cc5b2d/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f677261795f3664366436642e706e67" alt="Fork me on GitHub" data-canonical-src="https://s3.amazonaws.com/github/ribbons/forkme_right_gray_6d6d6d.png"></a>';
echo $output;
echo "</body></html>";

/* Save this request */
/* $fp = fopen("req.txt","w"); 	overwrite last request */
$fp = fopen("req.txt","a");
fwrite($fp, serialize($_GET));
fclose($fp);


