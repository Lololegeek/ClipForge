# clipforge design notes

dark theme, primary is blue/violet.

colors (oklch source -> qt sRGB)
- canvas: oklch(0.165 0.012 275) -> #101119
- nav: oklch(0.19 0.014 275) -> #171822
- surface: oklch(0.23 0.018 275) -> #20222f
- primary: oklch(0.66 0.17 277) -> #8c8cff
- text: oklch(0.96 0.006 275) -> #f0f1fa
- secondary: oklch(0.76 0.02 275) -> #b0b3c8

fonts
- segoe ui variable for text
- consolas for times and numbers
- body 14px, utility 12px, page titles 28px

spacing: 8px unit. corners 8-12px.

layout: fixed nav, library takes the rest, big video area.

bits
- replay ruler = horizontal bar, shows real buffer coverage + the F8 save. no fake thumbnails or numbers.
- cards only for real video thumbnails.
- selected control = tinted surface. focus = visible border.

motion: state changes 150ms, off when reduced motion is on.
