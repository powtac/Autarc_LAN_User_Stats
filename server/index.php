<?php

require_once dirname(__FILE__).'/basics.php';
require_once dirname(__FILE__).'/mysql.php';

header('content-type: text/html;charset=utf-8');

echo '<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
      "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="de">
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
        echo '<!--
        Letzter Request:<br />';
        print_r (unserialize(file_get_contents("req.txt")));
        echo '<br /><br /><br /> -->';
        echo 'Folgende Ger&aumlte sind online: <br /><br />';
        echo '<table border="0" width="50%">';
        echo '<tr><th>IP</th><th>MAC</th><th>Offline seit</th><th>Letzte &Uumlberpr&uumlfung: </th></tr>';
        $online_list = mysql_query("SELECT MAC, IP, time_offline, last_scan FROM Online_DB WHERE AVR_ID='".$AVR_ID."' ORDER BY IP ASC");
        while ($row = mysql_fetch_assoc($online_list))
        {
            $timediff = $date - $row['time_offline'];
            echo "<tr><td>".$row['IP']."</td><td><a href=\"http://www.coffer.com/mac_find/?string=".urlencode(mac_format($row['MAC'])).""\" target=\"_blank\">".mac_format($row['MAC'])."</a></td><td>".(date("j" , $timediff) - 1)." Tage ".(date("G" , $timediff) - 1)." h ".date("i" , $timediff)." min ".date("s" , $timediff)." sec</td><td>".date("Y-m-d - G:i" , $row['last_scan'])." Uhr</td></tr>";
        }
        echo '</table>';
    }
}
else {
    echo 'Bitte w&auml;hlen Sie eine AVR-ID aus:<br /><br />';
    echo '<form action="" method="GET" accept-charset="UTF-8"><select name='AVR_ID'>';
        $AVR_list = mysql_query("SELECT AVR_ID FROM Online_DB GROUP BY AVR_ID");
        while ($row = mysql_fetch_assoc($AVR_list))
        {
            echo '<option value="'.$row['AVR_ID'].'">&nbsp;'.$row['AVR_ID'].'&nbsp;</option>';
        }
    echo '</select><br /><br /><input type="submit" name="cmdShow" value="Anzeigen"/></form>';

}

echo '<a href="https://github.com/powtac/Autarc_LAN_User_Stats/"><img style="position: absolute; top: 0; right: 0; border: 0;" src="https://github-camo.global.ssl.fastly.net/a6677b08c955af8400f44c6298f40e7d19cc5b2d/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f677261795f3664366436642e706e67" alt="Fork me on GitHub" data-canonical-src="https://s3.amazonaws.com/github/ribbons/forkme_right_gray_6d6d6d.png"></a>';
echo "</body>
</html>";


/* Save this request */
$fp = fopen("req.txt","w");
fwrite($fp, serialize($_GET));
fclose($fp);


