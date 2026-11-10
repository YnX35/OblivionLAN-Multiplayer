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
- [ ] Compiler le dernier changement et inspecter le build.
- [ ] Tester le handshake depuis deux instances réelles d'Oblivion.
- [ ] Ajouter une file de messages bornée entre réseau et thread de jeu.
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

## Étape suivante — présence et position

Avant d'envoyer une position, identifier dans le SDK/runtime un mécanisme approprié pour obtenir les mises à jour sur le thread de jeu. Les paquets réseau devront être décodés en données simples, puis transférés par une file bornée. Aucune adresse mémoire ou fonction du moteur ne sera appelée directement depuis le thread réseau.

Les positions ne suffisent pas à créer un avatar distant : il faudra ensuite identifier une méthode documentée et sûre pour représenter un autre joueur sans corrompre les sauvegardes ni les références du monde.

## Sécurité

- Ne pas exposer le port à Internet.
- Garder des tailles de messages bornées et valider les valeurs.
- Tester sur des sauvegardes copiées.
- Les entités uniques et liées aux quêtes restent hors des règles génériques.
- Ne jamais marquer une étape comme validée sans résultat observé.
