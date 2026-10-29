// Liens sponsos : une ligne par créateur. Aucun pour l'instant : la route est
// désactivée (dossier app/_s), car l'export statique refuse une liste vide.
// Pour l'activer, ajouter un sponsor ici et renommer app/_s en app/s. Le lien à donner est
// https://lololegeek.github.io/ClipForge/s/<slug>/
// `slug` : minuscules, chiffres et tirets. Il sert de utm_source dans Umami.
export const SPONSORS = [] as const satisfies readonly { slug: string; name: string }[];

export type SponsorSlug = (typeof SPONSORS)[number]["slug"];
