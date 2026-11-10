# Plugin xOBSE — prototype de connexion LAN

## Fonctionnalités implémentées

- Mode=disabled : réseau désactivé.
- Mode=host : écoute sur le port TCP configuré et accepte jusqu'à trois clients distants.
- Mode=client : se connecte à l'adresse IPv4 configurée.
- Handshake OLAN version 1, avec en-tête et tailles de messages bornées.
- Le travail réseau s'exécute dans des threads séparés et n'appelle pas le moteur Oblivion.
- Journalisation dans OblivionLAN.log.
- Un client déconnecté libère son emplacement ; l'hôte continue d'écouter.

## Limites

Cette version ne synchronise aucun gameplay. Elle ne transmet pas les positions et ne crée pas d'avatar distant. Les messages de gameplay, l'interface sûre avec le thread du jeu, la reconnexion, les entités, les quêtes et la persistance ne sont pas implémentés.

Un build réussi ne prouve pas le chargement ni le fonctionnement en jeu. Tester sur des copies de sauvegardes et sur un réseau local privé uniquement.

## Configuration

Voir INSTALL.txt et OblivionLAN.ini.example.
