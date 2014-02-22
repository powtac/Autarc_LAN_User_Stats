-- phpMyAdmin SQL Dump
-- version 3.5.8.1
-- http://www.phpmyadmin.net
--
-- Host: localhost:3306
-- Erstellungszeit: 22. Feb 2014 um 15:47
-- Server Version: 5.1.72-2
-- PHP-Version: 5.3.10

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `danit_lan-user`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `Online_DB`
--

CREATE TABLE IF NOT EXISTS `Online_DB` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `AVR_ID` varchar(20) NOT NULL,
  `MAC` varchar(17) NOT NULL,
  `IP` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  `time_offline` int(11) NOT NULL,
  `last_scan` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=55 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
