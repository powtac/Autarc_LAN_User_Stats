<?php
header('Content-type: text/html; charset=utf-8');

mysql_connect('Server', 'Username', 'Password') or die (mysql_error());
mysql_select_db('danit_lan-user');
mysql_query('SET NAMES "utf8"');
mysql_set_charset("utf8");

?>