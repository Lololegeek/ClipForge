import Link from "next/link";
import { Logo } from "@/components/logo";
import { DISCORD_URL, DOCS_URL, REPO_URL, YOUTUBE_URL } from "@/lib/site";

export function SiteFooter() {
  return (
    <footer className="border-line text-muted border-t py-12 text-sm">
      <div className="mx-auto grid w-[min(72rem,100%-2rem)] items-center gap-6 sm:w-[min(72rem,100%-3rem)] md:grid-cols-[auto_1fr_auto] md:gap-10">
        <div className="text-ink flex items-center gap-2.5 text-lg font-semibold">
          <Logo size={26} />
          ClipForge
        </div>
        <p>Logiciel libre sous licence GPL-3.0-or-later. Aucune télémétrie, aucun compte.</p>
        <nav aria-label="Pied de page" className="flex flex-wrap gap-x-6 gap-y-2">
          <a className="hover:text-ink" href={REPO_URL} target="_blank" rel="noopener noreferrer">
            GitHub
          </a>
          <a className="hover:text-ink" href={DISCORD_URL} target="_blank" rel="noopener noreferrer">
            Discord
          </a>
          <a className="hover:text-ink" href={YOUTUBE_URL} target="_blank" rel="noopener noreferrer">
            YouTube
          </a>
          <Link className="hover:text-ink" href="/fonctionnalites/">
            Fonctionnalités
          </Link>
          <Link className="hover:text-ink" href="/changelog/">
            Changelog
          </Link>
          <Link className="hover:text-ink" href="/telechargement/">
            Télécharger
          </Link>
          <Link className="hover:text-ink" href="/licences/">
            Licences tierces
          </Link>
          <a
            className="hover:text-ink"
            href={DOCS_URL("docs/CAPTURE.md")}
            target="_blank"
            rel="noopener noreferrer"
          >
            Capture
          </a>
        </nav>
      </div>
    </footer>
  );
}
