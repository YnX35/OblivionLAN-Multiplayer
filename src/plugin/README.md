# Premier plugin xOBSE

## But

Cette étape ne fait qu'établir la chaîne de compilation et le chargement xOBSE. Aucun réseau, hook moteur, commande console, accès aux sauvegardes ou synchronisation n'est inclus.

## SDK

Le workflow utilise le dépôt officiel `llde/xOBSE`, épinglé sur le commit :

`5ad9b4a287a27801e6d01f22769e84d851509595`

Il réutilise le projet officiel `obse_plugin_example` et ses dépendances, mais remplace son point d'entrée par une implémentation minimale. Le projet est compilé en Win32/Release avec MSVC v143 sur le runner Windows 2022. Le projet SDK d'origine déclare v142 ; le workflow adapte les fichiers de projet à v143 pour utiliser le toolset fourni par le runner.

## Version ciblée

`OBLIVION_VERSION=0x010201A0`, conformément à la configuration du projet exemple officiel pour Oblivion 1.2.0.416. Le plugin refuse les autres versions et l'éditeur.

Cette valeur est une cible de compilation ; elle ne prouve pas à elle seule que l'exécutable français installé expose exactement le même runtime. Le test réel reste obligatoire.

## Récupérer le build

Dans GitHub :
1. Ouvrir l'onglet **Actions**.
2. Sélectionner **Build xOBSE plugin (Win32)**.
3. Ouvrir la dernière exécution verte.
4. Télécharger l'artifact **OblivionLAN-xOBSE-Win32**.
5. Extraire `OblivionLAN.dll` et suivre `INSTALL.txt`.

L'artifact est conservé 14 jours. Aucun binaire n'est commité dans le dépôt.

## Critères de réussite

- [ ] Le workflow compile la solution xOBSE en Win32/Release.
- [ ] L'artifact contient une DLL x86.
- [ ] xOBSE accepte le plugin dans Oblivion classique.
- [ ] Le jeu démarre et charge une sauvegarde de test sans crash.

Ne jamais marquer une étape comme validée avant d'avoir observé le résultat correspondant.
