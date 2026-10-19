# Website screenshots

The PNG files in `site/public/screenshots/` are unretouched 1320 x 840 captures of the running native Qt Quick application on Windows 11, taken on 19 October 2026. They are not HTML recreations. The application window ran outside the visible desktop, without taking focus.

The app uses the same logo as `site/public/favicon.svg`; that SVG is copied verbatim to `assets/icons/clipforge-mark.svg` and rendered at multiple sizes into `assets/ClipForge.ico` for Windows, the tray and the installer.

The library and editor contain three real 20-second MP4 excerpts imported from **Big Buck Bunny**, copyright Blender Foundation, https://peach.blender.org/, licensed under [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/). Excerpts start at 30, 120 and 240 seconds of the official 480p movie. This is sample video content, not a claim of game capture. Movie source: https://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_h264.mov.zip . The movie files stay local and are not shipped with ClipForge.

The `--ui-test --offscreen-window --data-dir <isolated-folder>` mode captures application pages into the selected folder. The `--editor-test` mode additionally checks a valid QVideoSink frame and an advancing playback position before saving `editor-playing.png`. Screenshots of the games page are excluded because that page enumerates the user's open window titles and executable paths. No desktop capture or private application contents are included in the website.
