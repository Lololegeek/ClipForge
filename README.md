# ClipForge

Capture de replay Windows native et hors ligne. C++20, Qt Quick, libobs, FFmpeg et SQLite. GPL-3.0-or-later.

Télécharge l'[installateur Windows x64](https://github.com/Lololegeek/ClipForge/releases/latest/download/ClipForge-Setup-x64.exe) ou le [ZIP portable](https://github.com/Lololegeek/ClipForge/releases/latest/download/ClipForge-Windows-x64.zip). L'installateur s'exécute pour l'utilisateur courant, sans droits administrateur. Consulte le [site](https://lololegeek.github.io/ClipForge/) pour les captures d'écran réelles de l'application et les détails des versions.

Lance `dist/ClipForge/ClipForge.exe`. Ouvre **Paramètres**, sélectionne l'écran ou la fenêtre, applique les réglages, puis active la relecture instantanée. Appuie sur **F8** (touche par défaut, modifiable) pour sauvegarder un clip des dernières secondes. Fermer la fenêtre laisse la capture tourner dans la barre des tâches par défaut ; utilise l'action Quitter de la barre des tâches pour quitter.

ClipForge vérifie les nouvelles versions sur GitHub (désactivable dans **Paramètres**) et, pour une installation via l'installateur, télécharge puis installe la mise à jour à la fermeture.

Une notification en jeu de 3 secondes confirme chaque clip sauvegardé (fenêtre ou plein écran sans bordure ; désactivable dans **Paramètres**). Les raccourcis sont protégés contre le spam : un clip à la fois, 1,5 s minimum entre deux demandes.

Utilise **Mes clips** pour lire une vidéo et ouvrir l'éditeur. Choisis les points de début et de fin, puis clique sur **Garder ce passage** pour couper ; **Ajouter** ajoute un autre passage. Exporte un nouveau MP4. Les originaux sont conservés. Les fichiers de relecture et les exports de l'éditeur sont en MP4 ; les anciens enregistrements MKV peuvent toujours être importés.

## Compilation

Installe Visual Studio avec MSVC x64 et un Windows SDK, CMake, Ninja, Python (téléchargement des dépendances uniquement), 7-Zip et FFmpeg/FFprobe. PowerShell 7 :

```powershell
cd 'D:\Code 2\LoloClips'
./scripts/bootstrap.ps1
./scripts/build.ps1
./scripts/package.ps1
./scripts/test.ps1
```

Les dépendances sont isolées dans `.deps`. Python n'est jamais utilisé par l'application. Voir [compilation](docs/BUILDING.md), [capture](docs/CAPTURE.md), [validation](docs/VALIDATION.md) et [inventaire des composants tiers](THIRD_PARTY.md) pour le périmètre pris en charge, les preuves et les archives sources.
