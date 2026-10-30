# Plugin xOBSE avec première connexion LAN

## État

Le plugin contient maintenant une couche réseau TCP optionnelle, activée par un fichier `OblivionLAN.ini` placé à côté de `OblivionLAN.dll`.

- `Mode=disabled` : réseau désactivé.
- `Mode=host` : écoute sur le port TCP configuré et accepte un client.
- `Mode=client` : se connecte à l'adresse IPv4 configurée.
- Le protocole est compatible avec le prototype autonome : en-tête `OLAN`, version 1, HELLO/WELCOME, tailles bornées.
- Le travail réseau s'exécute dans un thread distinct ; il n'appelle pas le moteur Oblivion.
- Le plugin maintient la connexion ouverte après le handshake et écrit les événements dans `OblivionLAN.log`.

## Limites importantes

Cette étape n'envoie aucune position de joueur et ne crée pas d'avatar distant. Le thread réseau ne doit pas être utilisé pour appeler des fonctions du moteur. La connexion intégrée doit être compilée et testée dans deux instances réelles d'Oblivion avant d'être considérée comme validée.

Le mode hôte n'accepte qu'un client. Les messages de gameplay, reconnexion, déconnexion propre côté jeu, synchronisation de cellules, combat, entités et sauvegardes ne sont pas encore implémentés.

## Configuration

Voir `INSTALL.txt` et `OblivionLAN.ini.example`. Commencer par `Mode=disabled` si un test réseau n'est pas souhaité.
