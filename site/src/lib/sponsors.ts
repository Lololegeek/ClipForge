// Liens sponsos : une ligne par créateur. Le lien à donner est
// https://lololegeek.github.io/ClipForge/s/<slug>/
// `slug` : minuscules, chiffres et tirets. Il sert de utm_source dans Umami.
export const SPONSORS = [
  { slug: "test", name: "Lien de test" },
] as const satisfies readonly { slug: string; name: string }[];

export type SponsorSlug = (typeof SPONSORS)[number]["slug"];
