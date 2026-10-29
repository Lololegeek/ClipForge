import "@fontsource-variable/jetbrains-mono";
import "@fontsource-variable/schibsted-grotesk";
import type { Metadata, Viewport } from "next";
import Script from "next/script";
import "./globals.css";
import { UMAMI_URL, UMAMI_WEBSITE_ID } from "@/lib/analytics";
import { Providers } from "@/components/providers";
import { SiteFooter } from "@/components/site-footer";
import { SiteHeader } from "@/components/site-header";

export const metadata: Metadata = {
  title: { default: "ClipForge · Replay et clips pour Windows", template: "%s · ClipForge" },
  description:
    "ClipForge garde les dernières secondes de ton jeu en mémoire. Appuie sur F8, le moment est sauvegardé. Hors ligne, sans compte, open source.",
  applicationName: "ClipForge",
  keywords: ["replay", "clips", "capture", "Windows", "OBS", "open source", "gaming"],
  verification: { google: "l9HqPO6qKN2gCfzcXpV-ZpkAOFiZaJDOqipo5eoXEyQ" },
};

export const viewport: Viewport = {
  themeColor: [
    { media: "(prefers-color-scheme: dark)", color: "#101119" },
    { media: "(prefers-color-scheme: light)", color: "#f6f6fb" },
  ],
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="fr" suppressHydrationWarning>
      <body>
        <Providers>
          <a
            href="#contenu"
            className="focus:bg-accent focus:text-accent-ink sr-only focus:not-sr-only focus:fixed focus:top-3 focus:left-3 focus:z-50 focus:rounded-lg focus:px-4 focus:py-2"
          >
            Aller au contenu
          </a>
          <SiteHeader />
          <main id="contenu">{children}</main>
          <SiteFooter />
        </Providers>
        {UMAMI_WEBSITE_ID && (
          <>
            <Script
              src={`${UMAMI_URL}/script.js`}
              data-website-id={UMAMI_WEBSITE_ID}
              strategy="afterInteractive"
            />
            {/* Relectures de session et heatmaps (échantillonnage réglé dans Umami). */}
            <Script
              src={`${UMAMI_URL}/recorder.js`}
              data-website-id={UMAMI_WEBSITE_ID}
              strategy="lazyOnload"
            />
          </>
        )}
      </body>
    </html>
  );
}
