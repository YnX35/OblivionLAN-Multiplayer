# Architecture proposée et état actuel

Cette architecture reste la cible globale. Une première tranche est déjà implémentée : plugin xOBSE Win32, transport TCP hôte/client, relais d'instantanés de position et tâche xOBSE Tasks2. Ces fonctions compilent, mais leur fonctionnement dans deux instances réelles d'Oblivion n'est pas encore validé. Un prototype d'avatar distant same-cell est présent et compilé ; son apparition, son déplacement réel et son cycle de vie en sauvegarde restent à valider. Aucun combat ni état de monde partagé n'est encore appliqué au moteur.

## 1. Vue d'ensemble

Deux couches sont envisagées :

1. Intégration jeu : plugin xOBSE x86 chargé dans Oblivion. Il observe les événements pertinents et applique les changements sur le thread approprié du jeu.
2. Couche réseau et état partagé : protocole indépendant du moteur, gestion de session, validation côté hôte et persistance des données du mod.

Schéma conceptuel :

    Client Oblivion A ─┐
                       ├─ LAN ─ Hôte Oblivion + autorité du monde
    Client Oblivion B ─┘                    |
                                           ├─ validation des actions
                                           ├─ état partagé
                                           └─ stockage persistant du mod

Dans une partie à deux joueurs, l'un des clients héberge aussi l'autorité. Aucun serveur dédié n'est requis pour le prototype.

## 2. Autorité et cohérence

L'hôte est seul responsable de valider les changements partagés. Un client peut demander une action ; il ne peut pas imposer directement un nouvel état.

Exemples à valider côté hôte : présence des joueurs, interaction avec un objet partagé, état d'une créature gérée par le mod et progression d'une quête explicitement prise en charge.

Les règles de résolution des conflits restent à spécifier. La synchronisation des quêtes est risquée : les scripts vanilla peuvent déclencher des effets locaux ou irréversibles. Commencer par un périmètre limité et testé.

## 3. Frontière réseau / moteur

Le thread réseau ne doit pas appeler les fonctions internes d'Oblivion.

Flux envisagé :
1. recevoir des octets ;
2. vérifier taille maximale et framing ;
3. décoder avec des conversions explicites ;
4. vérifier version, type, identifiants et valeurs ;
5. placer une commande validée dans une file bornée ;
6. traiter depuis le thread sûr du jeu ;
7. renvoyer le résultat validé par l'hôte.

Les files doivent être bornées. Une surcharge produit une erreur contrôlée ou une déconnexion, jamais une croissance mémoire illimitée.

## 3.1 Interface de commandes xOBSE

Le plugin interroge désormais l'interface `OBSEConsoleInterface` et vérifie sa version ainsi que le pointeur `RunScriptLine2`. Cette interface documentée permet d'exécuter une ligne de script avec une référence appelante. Toute utilisation doit rester sur la boucle principale du jeu ; aucun worker réseau ne doit l'appeler. La disponibilité de l'interface est détectée. Un prototype appelle `PlaceAtMe`, recherche la nouvelle référence dans la cellule et mémorise son FormID pour la résoudre via `LookupFormByID`. Ce chemin compile, mais sa sûreté, son rendu 3D, sa désactivation et son comportement dans les sauvegardes ne sont pas encore validés en jeu.

## 4. Protocole réseau

Le protocole n'est pas encore choisi. Spécifier avant implémentation :
- identifiant magique et version ;
- type de message ;
- longueur avec limite stricte ;
- identifiant de session et séquence ;
- encodage explicite des nombres et ordre des octets ;
- comportement face aux messages inconnus, incomplets, dupliqués ou hors ordre ;
- timeout et fermeture propre.

Ne pas envoyer directement des structs C++ sur le réseau : padding, alignement et représentation peuvent varier. Ne jamais faire confiance à une longueur annoncée par un pair.

Le choix TCP/UDP ou hybride reste ouvert. Le décider après un prototype réseau autonome et des mesures simples sur LAN.

## 5. Identifiants et entités du monde

Ne pas supposer que les FormID sont universellement identiques dans toutes les installations et avec tous les ordres de mods. Une stratégie d'identification partagée doit être définie et testée.

Chaque entité synchronisée devra avoir une catégorie explicite : joueur, créature ordinaire gérée par le mod, entité unique ou de quête, objet partagé, état de quête pris en charge.

Par défaut, une entité non reconnue ne doit pas être réinitialisée ou supprimée par le système multijoueur.

## 6. Persistance

La persistance du mod devra être séparée autant que possible des sauvegardes vanilla. SQLite est une option candidate, pas un choix définitif.

À spécifier : schéma versionné, transactions atomiques, sauvegarde/restauration, migrations, identité de session et du monde, comportement si la base est absente, verrouillée ou corrompue.

L'hôte est la source de vérité pour les données partagées. Les clients ne doivent pas pouvoir écraser directement la base de l'hôte.

## 7. Sécurité et robustesse

Le LAN n'est pas une frontière de confiance. Limiter tailles de paquets et files ; rejeter valeurs non finies ou hors limites ; limiter le débit ; ne jamais interpréter les données réseau comme du code ou des chemins ; ne pas exposer de port sur Internet ; ne pas écrire dans les fichiers du jeu à partir d'une donnée réseau ; journaliser sans secrets.

Voir SECURITY.md.

## 8. Décisions ouvertes

- Version exacte du runtime Oblivion et de xOBSE.
- Version et disposition du SDK.
- Compilateur/IDE compatible avec la machine.
- Bibliothèque réseau et modèle TCP/UDP.
- Format de sérialisation.
- Threading et mécanisme de file.
- Stratégie d'identité des références du monde.
- Stratégie de persistance et de sauvegarde.

Aucune de ces décisions n'est déjà validée.
