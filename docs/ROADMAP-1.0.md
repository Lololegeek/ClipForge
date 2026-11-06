# ClipForge 1.0 — feuille de route

Inventaire issu d'une relecture complète du code 0.9.0 (C++, QML, installeur, CI) et d'une comparaison avec Medal, ShadowPlay, Outplayed, SteelSeries Moments et OBS.

Priorités : **P0** = bloquant pour sortir de la bêta · **P1** = attendu d'une 1.0 · **P2** = 1.x.
Les éléments marqués *(à vérifier)* sont des risques probables qu'il faut d'abord reproduire.

---

## A. Bugs

### A1. Profils de jeu (cassés en pratique)
- **P0** `GameProfiles::detect()` n'applique un profil que si l'état est `Idle` ; avec `alwaysReplay` (défaut) l'état est toujours `Buffering` → aucun profil ne s'applique jamais.
- **P0** Le profil écrase `settings_` en mémoire, jamais restauré à la fermeture du jeu.
- **P0** La page Paramètres lit `config()` → elle affiche les valeurs du profil et « Appliquer » les enregistre comme réglages globaux.
- **P0** À la fermeture du jeu, `stop()` coupe le replay même si `alwaysReplay` est activé.
- **P1** Deux jeux avec profil ouverts en même temps : aucun arbitrage (le dernier détecté gagne).
- **P1** La page Jeux ne liste que les fenêtres ouvertes : impossible de voir/modifier un profil d'un jeu fermé.

### A2. Réglages
- **P0** `Settings::load` renvoie `defaults()` si **un seul** champ est invalide → perte silencieuse de tous les réglages et profils.
- **P0** Pas d'écriture atomique de `settings.json` (coupure pendant l'écriture = fichier vide → retour aux défauts).
- **P1** Pas de numéro de version de schéma ni de migration (indispensable avant de figer le format en 1.0).
- **P1** Import de réglages : aucune validation partielle, même problème de « tout ou rien ».

### A3. Bibliothèque
- **P1** Le jeu n'est renseigné que par les profils → la plupart des clips sont « Vidéo locale », le filtre par jeu est inutile.
- **P1** `ClipModel::upsert` fait `reload()` à chaque clip → liste et défilement réinitialisés pendant un scan.
- ✅ **P1** Les exports héritent du jeu du clip source (fait) ; les tags ne suivent pas encore.
- ✅ **P1** Pas de `PRAGMA user_version` : les colonnes sont ajoutées par `ALTER` au petit bonheur, aucune migration testée.
- **P2** `favoriteMany` construit du SQL par concaténation (sûr car entiers, mais à passer en requête préparée).
- **P2** Cache des miniatures jamais purgé.

### A4. Éditeur / export
- **P1** `loadProject` prend comme durée la fin du dernier segment au lieu de la durée réelle.
- ✅ **P1** Bouton « Annuler l'export » (fait).
- **P1** Passages non réordonnables (« exportés dans cet ordre » mais pas de glisser-déposer).
- **P1** Limite de taille (Mo) en une seule passe → fichiers régulièrement au-dessus de la cible (Discord refuse).
- **P2** Repli sur le `ffprobe` du PATH dans `selectClip` au lieu de `MediaJobs::tool`.

### A5. Capture / système *(à vérifier)*
- **P0** Comportement après **veille / reprise** : périphérique GPU perdu, replay figé ou en erreur sans relance.
- **P0** Changement de résolution / d'écran en cours de partie, écran débranché.
- **P1** Jeu lancé **en administrateur** : `game_capture` ne peut pas s'injecter, les raccourcis peuvent ne pas arriver → détecter et le dire clairement.
- **P1** Chemins non ASCII (ex. `C:\Users\Loévan\…`) dans tous les chemins FFmpeg / OBS / SQLite.
- **P1** Disque plein **pendant** une sauvegarde : fichier tronqué ajouté à la bibliothèque ?
- **P1** Dossier de clips sur un disque externe débranché au démarrage.
- **P2** Compteurs CPU `static` dans `ObsEngine::stats()`.

### A6. UI
- ✅ **P1** Info-bulle de la zone de notification figée au démarrage, ne suit pas un changement de touche ni l'état du replay.
- ✅ **P1** `"F" + (saveKey - 111)` recopié 6 fois en QML → libellé faux dès qu'on autorise d'autres touches.
- **P2** Taille et position de la fenêtre non mémorisées.

---

## B. Capture

- ✅ **P0** Résolution **native** (suivre la source) + mise à l'échelle de sortie ; aujourd'hui largeur/hauteur fixes.
- **P1** Choix de la **carte graphique** (`vi.adapter = 0` en dur → portables à double GPU capturent l'écran noir).
- ✅ **P1** Option capturer le curseur (toujours activé).
- **P1** Choix du **périphérique de sortie audio** (toujours « default »).
- **P1** **Pistes audio par application** : capturer Discord / navigateur sur une piste à part (`wasapi_process_output_capture`), pour couper les potes au montage.
- **P1** **Suppression de bruit du micro** (filtre RNNoise d'obs-filters, déjà dans libobs) + gate / gain.
- **P1** Écran « où est le jeu / le curseur » automatiquement en multi-écran.
- **P1** Réglage automatique au premier lancement (détection GPU → encodeur + débit recommandés).
- **P1** Contrôle de débit **CQP/VBR** en plus du CBR ; préréglages Qualité / Équilibré / Performance.
- **P2** Capture **HDR** (aujourd'hui rien de prévu ; image délavée sur écran HDR).
- **P2** Tampon sur disque pour des replays longs (10–30 min) sans exploser la RAM.
- **P2** Enregistrement **continu** (l'état `Recording` existe déjà) avec raccourci démarrer/arrêter.
- **P2** Webcam en incrustation.
- **P2** Mode économie sur batterie (baisser fps/débit, ou mettre en pause sur secteur débranché).

### Vie privée
- **P1** **Liste d'applications exclues** : mettre le replay en pause quand une app sensible est au premier plan (gestionnaire de mots de passe, banque).
- ✅ **P1** S'assurer que le toast ClipForge n'apparaît pas dans les captures (`WDA_EXCLUDEFROMCAPTURE`).

---

## C. Raccourcis

- **P0** Combinaisons **Ctrl/Alt/Maj + n'importe quelle touche** (aujourd'hui F1–F24 uniquement) et éditeur de raccourci « appuie sur une touche ».
- **P1** Signaler les conflits proprement (raccourci déjà pris par une autre app : quel raccourci, quoi faire).
- **P1** Boutons latéraux de souris.
- **P1** Raccourci **capture d'écran** (image PNG instantanée depuis le replay).
- **P1** Raccourcis « clip de 15 s / 30 s / 60 s » distincts.
- **P2** Hook clavier bas niveau en option pour les jeux plein écran exclusif.
- **P2** Manette (bouton Partage Xbox / combinaison).

---

## D. Bibliothèque

- **P0** Association **automatique au jeu** à la sauvegarde (exe au premier plan, nom lisible depuis `FileDescription`, icône extraite de l'exe).
- **P1** **Surveillance du dossier** (`QFileSystemWatcher`) au lieu du bouton Actualiser.
- **P1** **Glisser un clip** de ClipForge vers Discord / l'Explorateur (drag-out de fichier).
- ✅ **P1** **Copier le fichier** dans le presse-papiers (coller directement dans Discord) — aujourd'hui seulement « Copier le chemin ».
- ✅ **P1** Glisser-déposer des vidéos dans la bibliothèque pour les importer.
- **P1** **Aperçu au survol** (défilement de la miniature).
- **P1** Clips manquants : relocaliser le dossier ou purger en un clic.
- **P1** Vue groupée par jour / par session de jeu.
- **P1** Modèle de **nom de fichier** (`{jeu}_{date}_{heure}`) et sous-dossiers par jeu.
- **P2** Requêtes SQL hors du thread GUI.
- **P2** **Compression des vieux clips** (réencoder en HEVC/AV1 au-delà de N jours pour gagner de la place).
- **P2** Détection de doublons.
- **P2** Note / étoiles, collections.

---

## E. Éditeur et export

- **P0** **Préréglages d'export** : Discord (10 Mo), Discord Nitro (500 Mo), YouTube, Shorts/TikTok 9:16, GIF.
- **P1** Encodage **matériel** à l'export (NVENC/AMF/QSV) — aujourd'hui libx264/libx265 logiciel seulement.
- **P1** Export **rapide sans réencodage** quand il n'y a ni recadrage ni vitesse.
- **P1** Export **2 passes** pour respecter la taille cible.
- **P1** **File d'attente** d'exports + annulation + progression dans la barre des tâches (`ITaskbarList3`).
- **P1** **Forme d'onde** audio sur la timeline + zoom de la timeline.
- **P1** Fondu audio/vidéo en entrée/sortie.
- **P1** Couper le son d'un passage / volume par passage.
- **P1** Coupe rapide depuis la bibliothèque (garder les N dernières secondes sans ouvrir l'éditeur).
- **P2** **Montage multi-clips** (enchaîner plusieurs clips dans un export).
- **P2** Texte / titre, flou d'une zone (pseudos, chat).
- **P2** Rampe de vitesse (ralenti progressif).
- **P2** Option « supprimer l'original après export ».

---

## F. Partage

- **P1** Envoi vers un **webhook Discord** configurable en un clic.
- **P1** Bouton « Partager » après sauvegarde (toast cliquable : ouvrir / éditer / copier).
- **P2** Envoi YouTube / Streamable.

---

## G. Expérience et finitions

- **P0** **Assistant de premier lancement** : source, touche, dossier, test de capture avec aperçu.
- **P0** **Notifications Windows natives** (toast avec actions « Ouvrir », « Éditer ») en plus de l'overlay.
- **P1** Page **« Nouveautés »** affichée après une mise à jour (changelog intégré).
- **P1** Zone de notification : « Ouvrir le dernier clip », « Mettre en pause », icône différente si replay actif.
- **P1** **Jump list** de la barre des tâches (dernier clip, pause).
- **P1** Indicateur discret « replay actif » en jeu (optionnel, comme ShadowPlay).
- **P1** Position et durée du toast configurables, son personnalisable.
- **P1** Accessibilité : `Accessible.name` partout, navigation clavier complète, contrastes vérifiés.
- **P1** **Traduction anglaise** (`qsTr` partout + sélecteur de langue) — installeur aussi (aujourd'hui `french` uniquement).
- **P2** Thème clair.

---

## H. Fiabilité et diagnostic

- **P0** **Signature du code** (SignPath gratuit pour l'open source, ou certificat) : sans ça SmartScreen bloque l'installeur et beaucoup d'antivirus signalent l'injection `game_capture`.
- **P0** **Minidumps** en cas de plantage + bouton « Créer un rapport de diagnostic » (zip logs + réglages anonymisés + infos GPU).
- **P1** Bouton « Ouvrir les logs » dans Paramètres.
- **P1** Relance automatique du moteur OBS après plantage du thread de capture.
- **P1** Page d'aide « Compatibilité anti-triche » (Vanguard, EAC, BattlEye → bascule WGC).
- **P2** Rapport de plantage opt-in envoyé (toujours sans télémétrie par défaut).

---

## I. Qualité du code et tests

- **P0** **CI sur chaque push / PR** (build + tests) — aujourd'hui seul `release.yml` build, au moment du tag.
- **P1** Tests de régression : profils de jeu, réglages partiellement invalides, migration de la base, chemins Unicode, export rapide.
- **P1** Tests QML (`--ui-test` existe : l'étendre et le lancer en CI).
- **P1** Découper `EditorPage.qml` (918 lignes) et `SettingsPage.qml` (635) en composants.
- **P2** Nettoyer `main.cpp` (modes de test sur une ligne).
- **P2** `clang-format` + `qmlformat` vérifiés en CI.

---

## J. Distribution et projet

- **P0** Version 1.0.0 dans `CMakeLists.txt` et `packaging/ClipForge.iss`.
- **P0** Notes de version écrites à la main (aujourd'hui `--generate-notes`).
- **P1** Publication **winget** (et Scoop) automatisée depuis `release.yml`.
- **P1** Version **portable** officielle (le zip existe : lui faire stocker ses données à côté de l'exe).
- **P1** Canal de mises à jour **bêta** opt-in dans l'updater.
- **P1** Désinstalleur : proposer de supprimer ou garder réglages et bibliothèque.
- **P1** Mentions des licences tierces (OBS, FFmpeg, Qt) dans « À propos » — obligation GPL/LGPL.
- **P1** Docs : `FEATURES.md` (titre « Scope of 0.3 »), README, ARCHITECTURE, FAQ / dépannage.
- **P1** GitHub : modèles d'issue (bug / idée), `CONTRIBUTING.md`, `SECURITY.md`.
- **P1** Site : page 1.0, nouvelles captures, vidéo de présentation (`video-agent`), comparatif avec Medal/ShadowPlay.
- **P2** Bot Discord : rôle « annonces » pingé aux releases, salon partage de clips, formulaire de bug.
