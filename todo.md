Interner Ablauf:

Anahme: Subnet ist 255.255.255.0 => 255 mögliche hosts.

V0.01:

1. Alle Ips 1-255 pingen, die die antworten sind online
2. 1. Wiederholen, damit Liste aktuell bleibt

V0.1:

1. Subnet rausfinden
2. Anzahl Hosts im Netz mit Subnet berechnen
3. IPs des Netzes durchpingen, gefundene in array speichern
4. IP's wieder durchpingen, diesmal die IP's des array zuerst anpingen. Bei durchlauf Array aktualisieren
5. 4. Wiederholen
