import { z } from "zod";
import { REPO } from "./site";
import publishedRelease from "./release.json";

const releaseSchema = z.object({
  tag_name: z.string(),
  name: z.string().nullable().optional(),
  html_url: z.string().url(),
  published_at: z.string().nullable(),
  prerelease: z.boolean().default(false),
  assets: z.array(
    z.object({
      name: z.string(),
      size: z.number(),
      browser_download_url: z.string().url(),
    }),
  ),
});

export type Release = z.infer<typeof releaseSchema>;
export const RELEASE_SNAPSHOT: Release = releaseSchema.parse(publishedRelease);

/** Dernière release publiée, ou `null` si le dépôt n'en a aucune (404). */
export async function fetchLatestRelease(signal?: AbortSignal): Promise<Release | null> {
  const res = await fetch(`https://api.github.com/repos/${REPO}/releases/latest`, {
    headers: { Accept: "application/vnd.github+json" },
    signal,
  });
  if (res.status === 404) return null;
  if (!res.ok) throw new Error(`GitHub a répondu ${res.status}`);
  return releaseSchema.parse(await res.json());
}

export function pickInstaller(release: Release) {
  return (
    release.assets.find((a) => /setup.*\.exe$/i.test(a.name)) ??
    release.assets.find((a) => /\.(exe|msi)$/i.test(a.name)) ??
    release.assets.find((a) => /windows.*x64.*\.zip$/i.test(a.name)) ??
    null
  );
}
