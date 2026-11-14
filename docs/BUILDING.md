# Compilation et déploiement

Windows x64, outils compatibles MSVC 2022 ou plus récents, Windows SDK 10.0.22621 ou plus récent. La machine de test utilise Visual Studio 2026 (MSVC 19.51). Qt 6.8.3 est utilisé volontairement, à la place de l'installation Qt MinGW de la machine, qui est incompatible.

`bootstrap.ps1` télécharge les modules Qt MSVC officiels ainsi que la distribution portable et les sources officielles d'OBS 32.2.2. Le SHA256 du binaire OBS est vérifié. `build.ps1` charge l'environnement de développement Visual Studio, génère une bibliothèque d'import à partir des symboles exportés de l'obs.dll officiel, configure CMake et compile avec Ninja. Les en-têtes publics d'OBS viennent exactement de la même version des sources ; aucune déclaration inventée n'est utilisée.

`package.ps1` déploie dans `dist/ClipForge` les QML et plugins de Qt, les modules multimédia d'OBS, les outils de capture, les fichiers d'exécution D3D et les outils FFmpeg. `qt.conf` indique où se trouvent les plugins fournis. Les chemins d'OBS sont résolus à partir de l'exécutable de l'application, et non du dossier de lancement. L'outil d'assemblage des replays s'exécute depuis le dossier des binaires OBS fournis.

Le paquet inclut les DLL d'exécution MSVC propres à l'application. `package.ps1` construit aussi `dist/ClipForge-Setup-x64.exe` quand Inno Setup 6 est disponible. L'installation se fait pour l'utilisateur seul et propose des raccourcis facultatifs (bureau, démarrage), décochés par défaut. Aucun compte, droit administrateur ni activation en ligne n'est nécessaire.

Le dossier portable est autonome sur la machine de test ; une machine virtuelle Windows neuve et Windows 10 restent des vérifications obligatoires avant publication. `build.ps1 -Fresh` refait une compilation complète. Les kits de langue MSVC uniquement français demandent le préfixe showIncludes UTF-8 corrigé dans CMakeLists.txt pour que Ninja enregistre les dépendances des en-têtes ; cela se vérifie avec `ninja -C build -t deps`.
