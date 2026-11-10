# OblivionLAN-Multiplayer

Projet expérimental visant à créer un mode multijoueur LAN pour The Elder Scrolls IV: Oblivion classique (2006), pas Oblivion Remastered.

## État réel

- Plugin xOBSE Win32 compilé via GitHub Actions.
- OBSEPlugin_Load a déjà été atteint sur une installation de test.
- Le prototype réseau autonome a réussi un handshake entre deux PC sur le LAN.
- Le plugin intégré démarre un hôte TCP ou un client et effectue un handshake OLAN version 1.
- L'hôte peut accepter jusqu'à trois clients distants (quatre joueurs au total), avec un thread indépendant par connexion.
- Le dernier changement a été compilé avec succès en Win32 Release par GitHub Actions (build technique uniquement, pas un test en jeu).
- Une spécification OLAN v2 décrit les futurs paquets de présence et de position ; elle n'est pas encore implémentée.

## Limite majeure — transparence

Ce dépôt ne contient pas encore un véritable multijoueur jouable. La connexion réseau confirme seulement que les jeux peuvent établir une session TCP. Aucun avatar distant, mouvement, combat, créature, objet, inventaire ou quête n'est synchronisé. Le fait que deux jeux affichent « handshake successful » ne signifie pas qu'ils partagent le monde.

## Objectif final

Permettre à 2 à 4 joueurs sur le même réseau local de rejoindre une partie hébergée par un joueur, avec un monde partagé et persistant.

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
- Instructions : src/plugin/INSTALL.txt

Projet indépendant, non affilié à Bethesda.
