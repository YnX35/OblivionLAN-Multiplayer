# Protocole réseau OblivionLAN

## Objectif et limites

Ce document définit la prochaine évolution du protocole du projet pour **Oblivion classique 32 bits (2006)**. Il ne décrit pas OB:MP de ReadyM comme une dépendance : le projet ReadyM cible Oblivion Remastered et son environnement technique est différent.

La version actuellement intégrée au plugin utilise encore l'en-tête **OLAN v1**. Elle effectue le handshake TCP et transporte les `PLAYER_SNAPSHOT` dans les deux sens : les clients envoient leurs positions à l'hôte, qui les relaie ; l'hôte diffuse aussi sa propre position aux clients. Le code ajoute désormais l'envoi initial des instantanés connus à un nouvel arrivant ; cette dernière modification a été compilée avec succès en Win32 Release. Les instantanés distants sont validés puis déposés dans des emplacements bornés consultés par la boucle principale xOBSE ; le dernier état reste conservé pour initialiser les nouveaux clients, tandis que la boucle évite de retraiter le même numéro de séquence. Un premier prototype d'avatar distant same-cell est désormais présent dans le plugin : il tente de créer un proxy via `PlaceAtMe`, de retrouver sa référence et de la déplacer depuis la boucle principale. Le build Win32 Release du commit `e11322d` réussit, mais le rendu en jeu et le cycle de vie en sauvegarde ne sont pas encore testés. La spécification v2 ci-dessous reste une cible future.

## Transport

- Première cible : TCP sur le LAN, port configurable (47621 par défaut).
- Le TCP est conservé au début pour simplifier la validation et garantir l'ordre des messages.
- Les instantanés de position devront être limités en fréquence et regroupés/coalescés : envoyer des centaines d'anciennes positions en attente serait contre-productif.
- UDP ne sera évalué qu'après une mesure réelle de latence et de perte sur deux PC. Il ne faut pas ajouter un deuxième transport avant d'avoir un protocole testé.
- Le serveur est hébergé par un joueur ; aucune machine dédiée n'est nécessaire.
- Maximum visé : quatre joueurs au total, donc trois connexions distantes.

## En-tête de trame proposé — OLAN v2

Tous les entiers sont encodés en big-endian. Le récepteur doit rejeter toute trame dont la signature, la version, le type ou la longueur est invalide.

| Champ | Taille | Description |
|---|---:|---|
| Magic | 4 octets | ASCII OLAN |
| Version | 2 octets | 2 |
| Type | 2 octets | Type de message |
| Payload length | 4 octets | Longueur exacte, bornée avant allocation |
| Sequence | 4 octets | Numéro de séquence de l'émetteur |

La taille maximale doit être définie par type de message, pas seulement par une limite globale. Les valeurs flottantes doivent être vérifiées (pas de NaN ou d'infini) et les coordonnées doivent être bornées à une plage raisonnable.

## Types de messages proposés

| Type | Nom | Sens | But |
|---:|---|---|---|
| 1 | HELLO | Client → hôte | Version du protocole et nom court du joueur |
| 2 | WELCOME | Hôte → client | Identifiant de session et identifiant joueur attribué par l'hôte |
| 3 | PLAYER_SNAPSHOT | Bidirectionnel via l'hôte | Instantané d'un joueur |
| 4 | PLAYER_LEAVE | Hôte → clients | Retrait d'un joueur client de la session |
| 5 | PING | Bidirectionnel | Mesure de connexion |
| 6 | PONG | Bidirectionnel | Réponse au ping |
| 7 | ERROR | Hôte → client | Refus explicite et code d'erreur |

## Instantané joueur

Le premier instantané ne doit contenir que des données de lecture simples et vérifiables :

- identifiant réseau du joueur, attribué par l'hôte (jamais un FormID local utilisé comme identité réseau) ;
- numéro de séquence ;
- identifiant de cellule et, lorsque nécessaire, identifiant du worldspace ;
- position X/Y/Z ;
- rotation utile à l'orientation de l'avatar ;
- état de déplacement minimal si sa lecture est fiable ;
- horodatage monotone relatif à la session.

Les FormID du jeu sont des références locales à une installation et ne doivent pas être considérés comme des identifiants universels entre deux installations. Les cellules doivent être vérifiées côté hôte/client et les joueurs doivent être refusés ou isolés si leurs contenus ne sont pas compatibles.

## Architecture des threads

1. La boucle principale xOBSE lit l'état du joueur et construit un instantané.
2. Elle dépose l'instantané dans une file bornée et non bloquante.
3. Le worker réseau récupère le dernier instantané disponible et l'envoie.
4. Les messages entrants sont validés et déposés dans une file entrante bornée.
5. Seule la boucle principale du jeu peut appliquer un changement au moteur.
6. Si une file est pleine, les instantanés de position anciens sont remplacés/coalescés ; les événements fiables (déconnexion, état de quête, changement d'objet) auront une file et une politique distinctes.

Aucun appel au moteur Oblivion ne doit être effectué depuis un thread Winsock.

## Autorité et synchronisation

- L'hôte attribue les identifiants réseau et décide quels états partagés sont acceptés.
- Un client ne peut pas annoncer directement qu'une quête est terminée ou qu'un objet unique a changé.
- Les positions sont des instantanés, pas des commandes de téléportation. Le client devra interpoler les positions reçues lorsque l'avatar distant sera implémenté.
- Les données d'apparence (race, pièces d'équipement et mods requis) viendront dans un message distinct, après validation de la création d'avatar distant.
- Un identifiant d'objet ou de créature doit être un identifiant réseau stable attribué par la session, associé à une référence locale contrôlée ; ne jamais supposer que les FormID de deux sauvegardes sont interchangeables.

## Combat, magie et monde

Ces catégories sont volontairement exclues du premier paquet de synchronisation :

- Combat : événement validé par l'hôte, acteur réseau, cible réseau, type d'action, résultat et horodatage. Les dégâts ne doivent pas être appliqués deux fois.
- Magie : identifiant de sort/effet vérifié, lanceur, cible, position éventuelle, durée restante et fin d'effet. Les références de sorts doivent être compatibles entre installations.
- Objets/portes/activateurs : identifiant réseau stable, état avant/après, version de l'état et validation de l'hôte.
- Quêtes : événements explicites et liste blanche de quêtes testées. Ne pas répliquer aveuglément tous les scripts de quête.
- Créatures ordinaires : synchronisation séparée des créatures uniques et des acteurs de quête, avec règles de spawn/despawn et autorité hôte.

Le flag Persistent ne suffit pas à lui seul à rendre un objet synchronisable. Il faut aussi résoudre son identité, sa cellule, son état, sa durée de vie et sa sauvegarde.

## Sécurité et robustesse

- LAN uniquement dans les premières versions ; ne pas exposer le port à Internet.
- Limiter les tailles de messages, les fréquences et les allocations.
- Valider tous les champs avant de les transmettre au moteur.
- Gérer les déconnexions, les timeouts, les versions incompatibles et les séquences répétées.
- Ne jamais exécuter du code ou des scripts reçus du réseau.
- Tester sur des copies de sauvegardes.
- Une étape n'est validée qu'après compilation et test reproductible en jeu sur deux PC.

## État d'implémentation

- OLAN v1 : handshake TCP et premier paquet de position client → hôte, intégrés dans le code.
- xOBSE Tasks2 : capture périodique de la position locale et dépôt dans un instantané partagé entre threads.
- Hôte : validation de l'identifiant joueur, de la séquence, des coordonnées finies et de leur plage ; aucune application au moteur.
- Client : envoi du dernier instantané environ toutes les 100 ms ; les anciennes positions sont coalescées.
- Hôte et client : transfert du dernier instantané distant vers la boucle principale via quatre emplacements bornés (un par ID joueur distant, hôte inclus) ; les données sont lues/loguées, pas encore appliquées au moteur.
- Relais client → hôte → autres clients : implémenté dans le transport TCP.
- Diffusion de la position de l'hôte (ID 1) vers les clients : ajoutée et compilée avec succès. L'envoi initial au nouvel arrivant (position de l'hôte et derniers instantanés des clients déjà présents) est ajouté au code et compilé avec succès en Win32 Release.
- Message `PLAYER_LEAVE` lors de la déconnexion d'un client et nettoyage de son dernier instantané : ajoutés et compilés avec succès.
- OLAN v2 : spécification uniquement, pas encore implémentée.
- Avatar distant : prototype same-cell présent et compilé ; affichage, changement de cellule et sauvegardes non validés.
- Combat, NPC, objets, quêtes et persistance : pas encore implémentés.
