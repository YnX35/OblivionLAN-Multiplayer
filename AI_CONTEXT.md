# AI_CONTEXT.md — Contexte de référence

Lire ce document avant toute proposition ou modification du projet.

## Projet

Créer un mod multijoueur LAN pour The Elder Scrolls IV: Oblivion original (2006). Ne pas confondre avec Oblivion Remastered.

Objectif envisagé : 2 à 4 joueurs sur le même réseau local, une partie hébergée par un joueur, un état du monde partagé et persistant.

L'utilisateur débute en programmation. Expliquer simplement les étapes et ne pas supposer qu'il sait configurer un environnement C++.

## Contraintes non négociables

- Le jeu ciblé est Oblivion original, pas Remastered.
- Le jeu est historiquement un programme 32 bits. Toute DLL chargée dans le processus devra être compatible x86/Win32, même si Windows est 64 bits.
- xOBSE est l'extension envisagée. Ne jamais inventer de fonctions API : vérifier les en-têtes et exemples de la version exacte du SDK.
- Confirmer le runtime sur l'installation réelle ; ne pas supposer une version précise.
- La première cible est le LAN. Pas de serveur Internet ni d'exposition publique.
- 2 à 4 joueurs est un objectif, pas une capacité démontrée.
- Machine de développement : Windows 10 64 bits, 4 Go de RAM. Privilégier des outils légers.
- Stabilité et intégrité des sauvegardes prioritaires sur la vitesse.

## Architecture de principe

- Modèle hôte-autoritaire : l'hôte valide les changements d'état partagés.
- Les clients envoient des intentions/actions, pas des modifications arbitraires de l'état.
- Le thread réseau ne doit jamais appeler directement le moteur de jeu.
- Les messages reçus sont validés, bornés en taille et transférés vers le thread approprié.
- Le protocole est versionné et indépendant du moteur.
- Les données persistantes du mod sont séparées autant que possible des sauvegardes vanilla.
- Prévoir déconnexion, reconnexion, timeout, messages invalides et versions incompatibles.
- Les ennemis uniques et entités de quête ne sont jamais traités comme des mobs ordinaires.

Ces points sont des principes, pas une implémentation existante.

## Méthode obligatoire

Avant chaque changement :
1. Lire les fichiers concernés et leurs dépendances.
2. Vérifier les API dans le SDK exact et les documentations de référence.
3. Décrire hypothèses et risques.
4. Faire le changement le plus petit possible.
5. Ajouter ou adapter des tests.
6. Vérifier les résultats réels : compilation, tests et lancement, selon ce qui est possible.
7. Mettre à jour documentation et état des fonctionnalités.

Ne jamais affirmer « compilé », « testé », « sécurisé » ou « fonctionnel » sans preuve. Distinguer :
- Conçu : décrit mais pas codé.
- Implémenté : présent dans le code, pas nécessairement testé.
- Compilé : compilation réussie dans un environnement précisé.
- Testé : test exécuté avec résultat consigné.
- Validé en jeu : vérifié dans Oblivion réel avec les versions indiquées.

## Règles de code

- Utiliser un C++ compatible avec le compilateur et le SDK retenus.
- Pas de dépendance ajoutée sans justification, version fixée et instructions.
- Ne pas supposer de pragma ou d'API sans vérification.
- Éviter comportement indéfini, casts dangereux et accès mémoire non vérifiés.
- Valider tailles, longueurs, identifiants et valeurs réseau.
- Ne jamais désérialiser directement des structures C++ brutes depuis le réseau.
- Utiliser un format explicite avec ordre des octets défini.
- Ne pas faire d'allocation non bornée depuis des données reçues.
- Journaliser les erreurs sans données sensibles.

## Collaboration entre assistants

- Ce fichier est la source commune de vérité.
- Avant de modifier un fichier, vérifier son état actuel.
- Ne pas écraser le travail d'un autre contributeur.
- Utiliser une branche par fonctionnalité ou contributeur, puis une pull request vers main.
- Ne pas fusionner automatiquement du code non relu ou non testé.
- En cas de contradiction, le code et la documentation du SDK vérifiés priment ; corriger ce document ensuite.
- Ne jamais publier de clé API, token GitHub, mot de passe ou donnée personnelle.

## Hors périmètre initial

- Oblivion Remastered.
- Jeu en ligne public ou matchmaking Internet.
- Compatibilité garantie avec tous les mods.
- Synchronisation parfaite de toutes les quêtes dès la première version.
- Modification irréversible des sauvegardes vanilla.

## Références

- xOBSE : https://github.com/llde/xOBSE
- Guide : README.md
- Architecture : docs/ARCHITECTURE.md
- Roadmap : docs/ROADMAP.md
