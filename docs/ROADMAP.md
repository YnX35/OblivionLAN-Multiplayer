# Feuille de route

Une étape ne passe à « validée » qu'avec une preuve reproductible. « Implémenté » ne signifie pas « testé en jeu ».

## État actuel

- [x] Dépôt et documentation initiale.
- [x] Vérifier Oblivion classique en solo et consigner la version installée.
- [x] Compiler un plugin xOBSE Win32 minimal via GitHub Actions.
- [x] Confirmer sur l'installation de test que OBSEPlugin_Load est atteint.
- [x] Prototype réseau autonome compilé.
- [x] Handshake réseau autonome entre deux PC sur le LAN.
- [x] Ajouter au plugin une couche TCP optionnelle hôte/client avec configuration INI.
- [x] Étendre l'hôte pour accepter jusqu'à trois clients distants avec un thread par connexion.
- [x] Compiler le dernier changement et inspecter le build (GitHub Actions Win32 Release : succès).
- [x] Ajouter un point d'exécution récurrent sur la boucle principale via l'interface xOBSE Tasks2 (capture du joueur en lecture seule, diagnostic).
- [ ] Tester le handshake depuis deux instances réelles d'Oblivion.
- [x] Ajouter un instantané joueur partagé de manière thread-safe entre la boucle de jeu et le worker client (dernier état coalescé, pas de file non bornée).
- [x] conserver le dernier instantané de chaque joueur distant pour les arrivées tardives et éviter de retraiter une même séquence ; réinitialiser l'état à la déconnexion (compilation Win32 Release réussie).
- [x] Définir la spécification OLAN v2 (paquets joueur, séquences, limites, autorité hôte) ; spécification seulement, pas encore codée.
- [ ] Identifier et tester une méthode sûre pour lire l'état du joueur.
- [x] Implémenter le relais des snapshots clients depuis l'hôte vers les autres clients.
- [x] Diffuser la position de l'hôte vers les clients (compilation GitHub Actions Win32 Release réussie).
- [x] Notifier la déconnexion d'un client et nettoyer son instantané distant (compilation GitHub Actions Win32 Release réussie).
- [ ] Synchroniser et afficher deux joueurs dans une zone de test (transport, relais, transfert vers la boucle principale et proxy d'avatar codés ; test en jeu absent).
- [x] Compiler le prototype d'avatar distant same-cell basé sur `PlaceAtMe`, `LookupFormByID` et l'interface console xOBSE (GitHub Actions Win32 Release réussi sur le commit `e11322d`) ; inspection en jeu et sauvegardes toujours à tester.
- [ ] Valider le cycle de vie des avatars (réutilisation, désactivation, changement de cellule, fermeture et sauvegarde) sans accumulation de références.
- [ ] Tester déconnexion, fermeture du jeu, timeout et messages invalides.
- [ ] Synchroniser une interaction de monde réversible.
- [ ] Persistance contrôlée côté hôte.
- [ ] Gestion contrôlée des créatures ordinaires.
- [ ] Évaluer séparément la synchronisation des quêtes.

## Étape actuelle — connexions LAN

Le plugin lit OblivionLAN.ini dans Data/OBSE/Plugins. Le mode host écoute sur le port TCP configuré et accepte jusqu'à trois connexions. Le mode client se connecte à une IPv4. Le handshake vérifie OLAN version 1. Chaque connexion hôte est gérée par un thread séparé. Aucun thread réseau n'appelle le moteur Oblivion.

Cette étape est implémentée dans le code et les dernières modifications compilent en Win32 Release. Elle doit encore être testée dans deux instances réelles d'Oblivion.

## Étape actuelle — protocole gameplay

La spécification [docs/PROTOCOL.md](PROTOCOL.md) fixe les principes du prochain protocole : snapshots joueur, identifiants réseau attribués par l'hôte, files bornées entre réseau et jeu, validation stricte et autorité hôte. Le transport reste TCP au début ; UDP ne sera envisagé qu'après des mesures. Le protocole est partiellement implémenté pour les snapshots de position et les notifications de déconnexion ; les autres catégories d'état ne sont pas encore synchronisées.

## Étape actuelle — boucle principale xOBSE

Le plugin enregistre maintenant une tâche récurrente via `OBSETasks2Interface::EnqueueTaskRemovable`. Cette tâche s'exécute dans la boucle principale du jeu et lit, sans les modifier, la cellule, la référence de base et la position du joueur. Elle écrit un diagnostic toutes les cinq secondes. Ce point d'entrée est implémenté et compilé en Win32 Release ; son exécution doit encore être vérifiée dans Oblivion réel. Il ne synchronise pas encore les joueurs.

Le projet externe [ObMultiplayerServer](https://github.com/theturrell/ObMultiplayerServer) a été étudié comme référence fonctionnelle pour les catégories d'état (joueur, combat, quêtes, butin) et le relais. Son adaptateur de jeu utilise aussi des adresses mémoire runtime codées en dur et des commandes console expérimentales ; ces parties ne sont pas reprises sans vérification indépendante, car elles risqueraient de casser selon la version ou les sauvegardes.

## Étape suivante — relais et présence des joueurs

Le client capture un instantané sur la boucle principale et envoie le dernier état environ toutes les 100 ms. L'hôte vérifie l'identifiant attribué, le numéro de séquence et les coordonnées ; les paquets invalides ferment la connexion. L'hôte relaie les positions des clients, diffuse sa propre position, transmet les événements de déconnexion et envoie immédiatement sa dernière position connue à un client qui vient de rejoindre lorsque cet instantané est déjà disponible. Les snapshots sont validés puis transmis à la boucle principale via quatre emplacements bornés et coalescés. Ces changements passent la compilation Win32 Release, mais ils ne sont pas encore testés dans deux instances réelles. Ils ne sont pas encore appliqués au moteur et aucun avatar distant n'est affiché. La prochaine étape est une méthode sûre de création et de mise à jour d'avatar. Aucune fonction du moteur n'est appelée depuis un thread Winsock.

Un premier prototype de proxy d'avatar vient d'être ajouté : il tente de créer une copie du personnage local dans la même cellule, la découvre en comparant les références de la cellule avant/après `PlaceAtMe`, puis la déplace via des commandes xOBSE sur la boucle principale. Les références sont désormais conservées sous forme de FormID et résolues avec `LookupFormByID` avant utilisation, plutôt que de garder un pointeur brut à travers les changements de cellule. Le rendu reste limité à la même cellule. Le build Win32 du commit `e11322d` est réussi. L'affichage en jeu et le comportement des sauvegardes doivent encore être testés.

Les positions ne suffisent pas à créer un avatar distant : il faudra ensuite identifier une méthode documentée et sûre pour représenter un autre joueur sans corrompre les sauvegardes ni les références du monde. La commande script `PlaceAtMe` est documentée côté moteur, mais son invocation sûre depuis le plugin C++ n'est pas établie. Elle crée des références qui ne sont pas automatiquement supprimées ; une création répétée sans stratégie de réutilisation/nettoyage peut faire grossir les sauvegardes. Ne pas appeler une adresse moteur supposée ni créer des références brutes tant que le cycle de vie complet (création, enregistrement dans la cellule, 3D, suppression) n'est pas vérifié.

## Sécurité

- Ne pas exposer le port à Internet.
- Garder des tailles de messages bornées et valider les valeurs.
- Tester sur des sauvegardes copiées.
- Les entités uniques et liées aux quêtes restent hors des règles génériques.
- Ne jamais marquer une étape comme validée sans résultat observé.
