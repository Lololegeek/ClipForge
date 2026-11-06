# Checklist de test manuel – ClipForge 1.0 Beta Test 4

Coche chaque ligne. Note ce qui plante ou surprend.

## 0. Installation (idéalement sur un 2e PC ou une VM propre)
- [ ] Le setup s'installe sans erreur, raccourci Bureau/Menu Démarrer créés
- [ ] ClipForge se lance, pas de message « DLL manquante »
- [ ] Première ouverture : l'onboarding s'affiche, on peut le terminer
- [ ] Désinstallation propre depuis Paramètres Windows

## 1. Enregistrement de base
- [ ] Bouton Enregistrer : démarre, le timer tourne, le tray affiche l'état
- [ ] **Point de statut dans le tray** : change de couleur (inactif / enregistrement), l'infobulle correspond
- [ ] Arrêter / sauvegarder un clip : le fichier .mp4 apparaît dans la bibliothèque
- [ ] Le clip se lit avec son et image (écran + micro + son système)
- [ ] Résolution native (largeur = hauteur = 0) : le clip a bien la résolution de l'écran

## 2. Raccourcis clavier
- [ ] Dans Réglages : le champ de raccourci capture une touche seule
- [ ] Il capture aussi Ctrl+X, Alt+X, Shift+X et Ctrl+Alt+X
- [ ] Un raccourci invalide est refusé (message clair)
- [ ] Le raccourci fonctionne **dans un jeu plein écran**
- [ ] Après redémarrage de l'app, les raccourcis sont conservés

## 3. Overlay (en haut à gauche)
- [ ] Il s'affiche quelques secondes au lancement d'un jeu avec le rappel des touches
- [ ] Il apparaît quand un clip est sauvegardé
- [ ] Il n'apparaît **pas** dans les clips enregistrés
- [ ] Il ne vole pas le focus du jeu

## 4. Profils de jeu
- [ ] Lancer un jeu connu : le profil s'applique (qualité, durée…)
- [ ] Fermer le jeu : les réglages globaux reviennent (override temporaire)
- [ ] Les réglages globaux ne sont pas écrasés définitivement

## 5. Nom de fichier & dossiers par jeu
- [ ] Modèle `{jeu} {date} {heure}` : le nom du clip suit le modèle
- [ ] Dossier par jeu activé : le clip est rangé dans `<dossier>/<nom du jeu>/`
- [ ] Caractères interdits dans le nom d'un jeu (`:` `/` `?`) : pas de crash
- [ ] Deux clips à la même seconde : pas d'écrasement

## 6. Bibliothèque
- [ ] Glisser-déposer d'une vidéo externe : elle est importée
- [ ] Dossier surveillé : un fichier déposé apparaît après ~20 s (pas un fichier encore en cours d'écriture)
- [ ] Copier le fichier (menu contextuel) → coller dans Discord/Explorateur fonctionne
- [ ] Supprimer un clip : disparaît de la liste et du disque
- [ ] Clic droit → **Envoyer sur Discord** (avec un webhook de test) : message + vidéo reçus
- [ ] Discord : webhook invalide → message d'erreur, pas de crash

## 7. Éditeur / export
- [ ] Ouvrir un clip, lecture, plein écran, mode watch
- [ ] Trim : export MP4 de la bonne durée
- [ ] **Watermark** : l'image apparaît au bon endroit sur l'export
- [ ] **Normaliser le volume** : le son exporté est homogène (pas de coupure/distorsion)
- [ ] Export **GIF** : s'ouvre, couleurs correctes
- [ ] Export **WebP animé** : s'ouvre dans le navigateur
- [ ] Bouton **Annuler** pendant un export : s'arrête, pas de fichier à moitié écrit
- [ ] Un export garde le nom du jeu du clip d'origine
- [ ] Capture d'une image (snapshot PNG) et miniatures de la bibliothèque

## 8. Réglages
- [ ] Option curseur : curseur visible / masqué dans le clip
- [ ] Changer le dossier des clips : les nouveaux clips y vont
- [ ] Dossier de clips introuvable (disque débranché) : l'app ne plante pas
- [ ] Redémarrage : tous les réglages sont conservés
- [ ] Fichier de réglages corrompu à la main : l'app démarre quand même (réparation champ par champ)

## 9. Machine sans GPU récent / petite config (important car on a remis opengl32sw.dll)
- [ ] L'interface s'affiche correctement
- [ ] Pas d'erreur « Initialisation Direct3D impossible »
- [ ] Encodeur logiciel (x264) utilisable si NVENC/AMF/QSV absents

## 10. Mise à jour
- [ ] Installer ce setup **par-dessus** une ancienne version : clips et réglages conservés
