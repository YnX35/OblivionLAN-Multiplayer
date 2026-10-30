# Contribuer au projet

La priorité est la reproductibilité et la protection des sauvegardes.

## Avant de coder

1. Lire README.md, AI_CONTEXT.md et les documents d'architecture.
2. Vérifier la version du jeu, de xOBSE et du SDK ciblés.
3. Décrire le problème ou la fonctionnalité dans une issue.
4. Vérifier toute API xOBSE dans le SDK exact.

## Branches et pull requests

- Ne pas développer directement sur main.
- Créer une branche descriptive, par exemple feature/protocol-framing ou fix/packet-length-check.
- Une pull request explique le but, les fichiers modifiés, les tests exécutés et les limites connues.
- Une seule fonctionnalité cohérente par pull request.
- Ne pas fusionner une modification qui casse les tests ou change le protocole sans mise à jour de version.

## Qualité et validation

Préciser les validations réellement effectuées :
- revue statique ;
- compilation (configuration, architecture et version de l'outil) ;
- tests unitaires ;
- test LAN sur deux instances ;
- test en jeu ;
- test de sauvegarde/rechargement.

Ne pas déclarer une validation qui n'a pas été exécutée. Si un test est impossible, l'indiquer.

## Style

- Utiliser des noms explicites et cohérents.
- Documenter les invariants et les choix non évidents.
- Garder les fonctions courtes et gérer explicitement les erreurs.
- Justifier toute nouvelle dépendance.
- Ne pas stocker de secrets dans le code ou les issues.

## Fichiers interdits

Ne pas ajouter d'exécutables, DLL compilées, fichiers du jeu ou ressources Bethesda, sauvegardes personnelles, mots de passe, tokens, clés privées ou dumps mémoire contenant des données personnelles.

## Checklist de pull request

- [ ] Le changement répond à un objectif précis.
- [ ] Les hypothèses sont documentées.
- [ ] Les entrées externes sont validées.
- [ ] Les erreurs et déconnexions sont gérées.
- [ ] Les tests pertinents ont été exécutés.
- [ ] Les résultats réels et limites sont indiqués.
- [ ] La documentation est à jour.
- [ ] Aucun secret, binaire ou fichier du jeu n'est inclus.
