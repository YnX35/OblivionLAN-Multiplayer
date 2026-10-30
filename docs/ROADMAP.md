# Feuille de route

Une étape ne passe à « validée » qu'avec une preuve reproductible. « Implémenté » ne signifie pas « testé en jeu ».

## État actuel

- [x] Dépôt et documentation initiale.
- [x] Vérifier Oblivion classique en solo et consigner la version installée.
- [x] Compiler un plugin xOBSE Win32 minimal via GitHub Actions.
- [x] Confirmer sur l'installation de test que `OBSEPlugin_Load` est atteint.
- [x] Prototype réseau autonome compilé.
- [x] Handshake réseau autonome entre deux PC sur le LAN.
- [x] Ajouter au plugin une couche TCP optionnelle hôte/client avec configuration INI.
- [ ] Compiler et inspecter le build du plugin avec réseau intégré.
- [ ] Tester le handshake depuis deux instances réelles d'Oblivion.
- [ ] Gérer plusieurs clients (objectif final : jusqu'à trois clients plus l'hôte).
- [ ] Concevoir une interface sûre entre mises à jour du jeu et thread réseau.
- [ ] Synchroniser présence puis position de deux joueurs dans une zone de test.
- [ ] Tester déconnexion, fermeture du jeu, timeout et messages invalides.
- [ ] Synchroniser une interaction de monde réversible.
- [ ] Persistance contrôlée côté hôte.
- [ ] Gestion contrôlée des créatures ordinaires.
- [ ] Évaluer séparément la synchronisation des quêtes.

## Étape actuelle — Connexion réseau intégrée

Le plugin lit `OblivionLAN.ini` dans `Data\OBSE\Plugins`. Le mode `host` écoute sur le port TCP configuré ; `client` se connecte à une IPv4. Le handshake réutilise le format du prototype autonome. La connexion reste ouverte jusqu'à la fermeture du pair. Le thread réseau n'appelle aucune fonction du moteur.

Cette étape est **implémentée dans le code mais pas encore validée par un build et un test en jeu**.

## Étape suivante — Présence et position

Avant d'envoyer une position, identifier dans le SDK/runtime un mécanisme approprié pour obtenir les mises à jour sur le thread de jeu. Les paquets réseau devront être décodés en données simples, puis transférés par une file bornée. Aucune adresse mémoire ou fonction du moteur ne sera appelée directement depuis le thread réseau.

Les positions ne suffisent pas à créer un avatar distant : il faudra ensuite identifier une méthode documentée et sûre pour représenter un autre joueur sans corrompre les sauvegardes ni les références du monde.

## Sécurité

- Ne pas exposer le port à Internet.
- Garder des tailles de messages bornées et valider les valeurs.
- Tester sur des sauvegardes copiées.
- Les entités uniques et liées aux quêtes restent hors des règles génériques.
- Ne jamais marquer une étape comme validée sans résultat observé.
