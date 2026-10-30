# OblivionLAN-Multiplayer

Projet expérimental visant à créer un mode multijoueur LAN pour The Elder Scrolls IV: Oblivion original (2006).

> État actuel : documentation initiale. Aucun plugin jouable, serveur, client réseau ou système de synchronisation n'est implémenté. Aucun build n'est déclaré fonctionnel.

## Objectif

Permettre à 2 à 4 joueurs sur le même réseau local de rejoindre une partie hébergée par l'un d'eux, avec un monde partagé et persistant.

Principes visés :
- un hôte faisant autorité sur l'état partagé ;
- synchronisation progressive des joueurs ;
- progression commune définie explicitement ;
- persistance des données gérées par le mod ;
- contrôle des réapparitions des créatures ordinaires, sans réinitialiser arbitrairement les ennemis uniques ou liés aux quêtes.

## Périmètre

- Jeu ciblé : Oblivion original, pas Oblivion Remastered.
- Plateforme initiale : Windows 10 64 bits.
- Le jeu est un programme 32 bits ; le plugin chargé dans le jeu devra respecter l'architecture x86 et l'ABI du runtime.
- Extension envisagée : xOBSE. Les versions du jeu, de xOBSE, du SDK et du compilateur devront être figées avant le premier plugin.
- Réseau : LAN uniquement dans la première version.

## État et prochaines étapes

Consulter docs/ROADMAP.md, docs/ARCHITECTURE.md et AI_CONTEXT.md.

1. Vérifier le lancement du jeu sans mod.
2. Fixer précisément les versions du jeu et de xOBSE.
3. Construire et tester un plugin xOBSE minimal, sans réseau.
4. Définir et tester le protocole réseau indépendamment du jeu.
5. Synchroniser deux joueurs dans une zone de test.
6. Ajouter progressivement les systèmes persistants.

## Règles importantes

- Ne jamais présenter une fonctionnalité comme terminée sans test reproductible.
- Ne jamais remplacer les fichiers du jeu original sans sauvegarde.
- Ne jamais inclure de fichiers du jeu, de DLL compilées, de mots de passe ou de jetons dans le dépôt.
- Toute modification du monde doit être validée côté hôte.
- Les changements risquant d'altérer les sauvegardes doivent être testés sur des copies.

## Documentation

- Contexte commun aux assistants IA : AI_CONTEXT.md
- Contribuer : CONTRIBUTING.md
- Architecture : docs/ARCHITECTURE.md
- Feuille de route : docs/ROADMAP.md
- Sécurité : SECURITY.md

## Références techniques

- xOBSE : https://github.com/llde/xOBSE
- Documentation de création de plugins : https://github.com/llde/xOBSE#creating-obse-plugins

Projet indépendant, non affilié à Bethesda.
