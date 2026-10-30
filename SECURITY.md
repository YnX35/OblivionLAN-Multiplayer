# Sécurité et signalement

## Modèle de menace

Le projet vise d'abord un réseau local. Un réseau local peut néanmoins contenir des appareils non fiables ou des logiciels compromis. Les messages réseau sont des entrées non fiables.

## Règles minimales

- N'exposer aucun service sur Internet dans la version initiale.
- Ne pas demander de désactiver antivirus ou pare-feu.
- N'accepter que les messages attendus, bornés et validés.
- Limiter tailles de paquets, files et débit.
- Ne jamais exécuter, charger ou écrire un fichier indiqué par un paquet réseau.
- Ne pas inclure de tokens, mots de passe, clés ou données personnelles dans le dépôt.
- Tester les changements sur des copies de sauvegardes.
- Prévoir une désactivation propre du mod et une récupération après arrêt inattendu.

## Signalement

Pour un problème non sensible, ouvrir une issue avec étapes de reproduction, versions et extrait de journal expurgé.

Ne pas publier de sauvegarde personnelle, dump mémoire, token, mot de passe ou donnée privée dans une issue.
