# Site ClipForge

Next.js (App Router) exporté en statique : aucun serveur Node en production.

```bash
npm ci
npm run dev        # http://localhost:3000
npm run build      # génère out/
npm run start      # sert out/ sur http://localhost:3000
```

Les pages `/fonctionnalites`, `/telechargement` et `/licences` lisent `docs/*.md` et `THIRD_PARTY.md` du dépôt au moment du build.

GitHub Pages : activer *Settings → Pages → Source : GitHub Actions*. Le workflow `.github/workflows/site.yml` définit `NEXT_PUBLIC_BASE_PATH` sur le nom du dépôt. Sur un domaine à la racine, ne pas définir cette variable.
