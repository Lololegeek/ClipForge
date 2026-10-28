// Umami auto-hébergé. Laisser UMAMI_WEBSITE_ID vide désactive tout suivi.
export const UMAMI_URL = "https://unami.assetflow-website.com";
export const UMAMI_WEBSITE_ID = "11764eaa-74c1-4c14-8cfb-fa9000d0b452";

const SOURCE_KEY = "cf-sponsor";

type Umami = { track: (event: string, data?: Record<string, string>) => Promise<unknown> };
declare global {
  interface Window {
    umami?: Umami;
  }
}

/** Le sponsor par lequel le visiteur est arrivé, gardé jusqu'au prochain lien sponso. */
export function getSponsor(): string {
  try {
    return localStorage.getItem(SOURCE_KEY) ?? "direct";
  } catch {
    return "direct";
  }
}

export function setSponsor(slug: string) {
  try {
    localStorage.setItem(SOURCE_KEY, slug);
  } catch {}
}

/** Attend le script Umami (au plus `timeout` ms), puis envoie l'événement. Ne rejette jamais. */
export async function track(event: string, data?: Record<string, string>, timeout = 1500) {
  const start = Date.now();
  while (!window.umami && Date.now() - start < timeout) {
    await new Promise((r) => setTimeout(r, 50));
  }
  if (!window.umami) return;
  const left = Math.max(300, timeout - (Date.now() - start));
  await Promise.race([
    window.umami.track(event, data).catch(() => {}),
    new Promise((r) => setTimeout(r, left)),
  ]);
}
