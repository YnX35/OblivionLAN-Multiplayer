# OblivionLAN-Multiplayer

Projet expérimental visant à créer un mode multijoueur LAN pour The Elder Scrolls IV: Oblivion original (2006), pas Oblivion Remastered.

## État réel

- Le plugin xOBSE Win32 minimal a été compilé via GitHub Actions et son point d'entrée `OBSEPlugin_Load` a été confirmé sur l'installation de test de l'utilisateur.
- Le prototype réseau autonome a réussi un handshake entre deux PC sur le LAN.
- Une première connexion TCP optionnelle a maintenant été ajoutée au plugin xOBSE : mode hôte ou client via `OblivionLAN.ini`.
- **La nouvelle version intégrée doit encore être compilée et testée en jeu sur les deux PC.**
- Aucun personnage visible, mouvement, combat, créature ou quête n'est encore synchronisé.

## Prochaine validation

1. Compiler le plugin avec la nouvelle couche réseau.
2. Installer le même build sur les deux PC.
3. Configurer un PC en `Mode=host` et l'autre en `Mode=client`.
4. Vérifier `OblivionLAN.log` sur les deux PC et confirmer que le handshake fonctionne depuis les deux jeux.
5. Ensuite, rechercher une voie sûre pour obtenir les mises à jour du joueur sur le thread du jeu et transmettre une première donnée de présence.

## Objectif final

Permettre à 2 à 4 joueurs sur le même réseau local de rejoindre une partie hébergée par un joueur, avec un monde partagé et persistant. Cet objectif n'est pas encore atteint.

Principes visés :
- l'hôte valide les changements d'état partagés ;
- synchronisation progressive des joueurs ;
- progression commune définie explicitement ;
- persistance des données gérées par le mod ;
- contrôle des réapparitions des créatures ordinaires, sans réinitialiser arbitrairement les ennemis uniques ou liés aux quêtes.

## Contraintes

- Oblivion classique 32 bits : le plugin est compilé en x86/Win32.
- xOBSE est l'extension retenue pour le prototype.
- LAN uniquement, sans serveur public.
- Stabilité et intégrité des sauvegardes prioritaires.

## Documentation

- Contexte commun : AI_CONTEXT.md
- Contribuer : CONTRIBUTING.md
- Architecture : docs/ARCHITECTURE.md
- Feuille de route : docs/ROADMAP.md
- Sécurité : SECURITY.md
- Instructions plugin : src/plugin/INSTALL.txt

Projet indépendant, non affilié à Bethesda.
