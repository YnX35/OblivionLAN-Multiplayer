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
- [ ] Ajouter une file de messages bornée entre réseau et thread de jeu.
- [x] Définir la spécification OLAN v2 (paquets joueur, séquences, limites, autorité hôte) ; spécification seulement, pas encore codée.
- [ ] Identifier et tester une méthode sûre pour lire l'état du joueur.
- [ ] Synchroniser la présence puis la position de deux joueurs dans une zone de test.
- [ ] Créer un avatar distant sans corrompre les sauvegardes ni les références.
- [ ] Tester déconnexion, fermeture du jeu, timeout et messages invalides.
- [ ] Synchroniser une interaction de monde réversible.
- [ ] Persistance contrôlée côté hôte.
- [ ] Gestion contrôlée des créatures ordinaires.
- [ ] Évaluer séparément la synchronisation des quêtes.

## Étape actuelle — connexions LAN

Le plugin lit OblivionLAN.ini dans Data/OBSE/Plugins. Le mode host écoute sur le port TCP configuré et accepte jusqu'à trois connexions. Le mode client se connecte à une IPv4. Le handshake vérifie OLAN version 1. Chaque connexion hôte est gérée par un thread séparé. Aucun thread réseau n'appelle le moteur Oblivion.

Cette étape est implémentée dans le code ; elle doit encore être compilée après la dernière modification et testée dans deux instances réelles d'Oblivion.

## Étape actuelle — protocole gameplay

La spécification [docs/PROTOCOL.md](PROTOCOL.md) fixe les principes du prochain protocole : snapshots joueur, identifiants réseau attribués par l'hôte, files bornées entre réseau et jeu, validation stricte et autorité hôte. Le transport reste TCP au début ; UDP ne sera envisagé qu'après des mesures. Cette spécification n'ajoute pas encore de synchronisation réelle.

## Étape actuelle — boucle principale xOBSE

Le plugin enregistre maintenant une tâche récurrente via `OBSETasks2Interface::EnqueueTaskRemovable`. Cette tâche s'exécute dans la boucle principale du jeu et lit, sans les modifier, la cellule, la référence de base et la position du joueur. Elle écrit un diagnostic toutes les cinq secondes. Ce point d'entrée est implémenté dans le code mais doit encore être compilé puis vérifié dans Oblivion réel. Il ne synchronise pas encore les joueurs.

Le projet externe [ObMultiplayerServer](https://github.com/theturrell/ObMultiplayerServer) a été étudié comme référence fonctionnelle pour les catégories d'état (joueur, combat, quêtes, butin) et le relais. Son adaptateur de jeu utilise aussi des adresses mémoire runtime codées en dur et des commandes console expérimentales ; ces parties ne sont pas reprises sans vérification indépendante, car elles risqueraient de casser selon la version ou les sauvegardes.

## Étape suivante — présence et position

Capturer un instantané joueur à intervalle borné sur la boucle principale, l'encoder dans le protocole réseau avec des champs explicites, puis le transmettre par le worker réseau. Les instantanés entrants devront passer par une file bornée avant toute application sur la boucle du jeu. Aucune adresse mémoire ou fonction du moteur ne sera appelée directement depuis le thread réseau.

Les positions ne suffisent pas à créer un avatar distant : il faudra ensuite identifier une méthode documentée et sûre pour représenter un autre joueur sans corrompre les sauvegardes ni les références du monde.

## Sécurité

- Ne pas exposer le port à Internet.
- Garder des tailles de messages bornées et valider les valeurs.
- Tester sur des sauvegardes copiées.
- Les entités uniques et liées aux quêtes restent hors des règles génériques.
- Ne jamais marquer une étape comme validée sans résultat observé.
