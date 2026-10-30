# Feuille de route

Les étapes sont séquentielles. Une étape ne passe à « validée » qu'avec une preuve reproductible.

## État actuel

- [x] Dépôt et documentation initiale.
- [ ] Vérifier Oblivion original en solo, sans mod.
- [ ] Identifier précisément l'exécutable/runtime installé.
- [ ] Choisir et documenter la version xOBSE/SDK correspondante.
- [ ] Préparer l'environnement C++ x86 compatible.
- [ ] Compiler et charger un plugin xOBSE minimal.
- [ ] Prototype de protocole réseau autonome.
- [ ] Connexion LAN entre deux processus de test.
- [ ] Synchronisation de présence et position de deux joueurs.
- [ ] Tests de déconnexion/reconnexion et messages invalides.
- [ ] Prototype d'état persistant géré par le mod.
- [ ] Synchronisation limitée d'une interaction de monde.
- [ ] Gestion contrôlée des créatures ordinaires.
- [ ] Évaluation séparée de la synchronisation des quêtes.

## Étape 0 — Baseline du jeu

Objectif : établir une installation de référence.

Critères :
- le jeu démarre sans xOBSE ni nouveau mod ;
- une nouvelle partie peut être lancée ;
- sauvegarde et rechargement fonctionnent ;
- la version exacte est consignée ;
- les sauvegardes de test sont séparées des sauvegardes personnelles.

## Étape 1 — Plugin minimal

Objectif : prouver que l'environnement de build et le chargement xOBSE fonctionnent.

Critères :
- architecture x86/Win32 ;
- versions du runtime, SDK et compilateur consignées ;
- plugin basé sur l'exemple/API du SDK exact ;
- initialisation et arrêt sans crash ;
- journal minimal confirmant le chargement ;
- test en jeu sur installation de test.

Aucun réseau ni accès aux entités du monde à ce stade.

## Étape 2 — Protocole autonome

Objectif : tester le protocole sans Oblivion.

Critères :
- format documenté et versionné ;
- limites de taille strictes ;
- décodage défensif ;
- tests des messages tronqués, trop longs, inconnus et mal formés ;
- aucun accès mémoire hors limites ;
- test client/serveur local reproductible.

## Étape 3 — LAN à deux joueurs

Objectif : établir une session LAN et afficher la présence des pairs.

Critères :
- connexion entre deux PC/processus du même réseau local ;
- validation côté hôte ;
- timeout et déconnexion propres ;
- aucun appel au moteur depuis le thread réseau ;
- aucun port exposé volontairement à Internet.

## Étape 4 — Synchronisation en jeu

Objectif : synchroniser une donnée limitée et réversible, puis les positions.

Critères :
- deux joueurs visibles dans une zone de test ;
- fréquence de mise à jour mesurée ;
- interpolation et correction définies ;
- transition de cellule testée séparément ;
- erreurs réseau sans crash ni corruption de sauvegarde.

## Étape 5 — Monde partagé persistant

Objectif : partager une petite interaction de monde avec stockage côté hôte.

Critères :
- données versionnées ;
- transactions et sauvegardes testées ;
- reconnexion et rechargement cohérents ;
- entités inconnues ou uniques protégées par défaut.

## Étape 6 — Créatures et quêtes

Ordre :
1. créatures ordinaires explicitement identifiées ;
2. réapparition gérée par le mod ;
3. interactions de quête limitées ;
4. tests de conflits et de sauvegarde.

Les ennemis uniques et entités de quête sont exclus des règles génériques de réapparition. La synchronisation générale des quêtes n'est pas promise.

## Journal des validations

Pour chaque étape, consigner date, versions, machine, commandes, résultat, logs utiles et limites connues. Ne pas marquer une case terminée sur la base d'une simple revue théorique.
