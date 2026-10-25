import { Github, Windows } from "@/components/icons";
import Link from "next/link";
import { ReplayRuler } from "@/components/replay-ruler";
import { Button } from "@/components/ui/button";
import { Kbd } from "@/components/ui/kbd";
import { REPO_URL } from "@/lib/site";

export function Hero() {
  return (
    <section className="pt-14 pb-20 sm:pt-20">
      <div className="mx-auto w-[min(72rem,100%-2rem)] sm:w-[min(72rem,100%-3rem)]">
        <p className="text-muted mb-7 flex items-center gap-2.5 text-sm">
          <span className="bg-ok size-2 rounded-full shadow-[0_0_0_4px_color-mix(in_oklab,var(--ok)_16%,transparent)]" />
          Windows · hors ligne · sans compte · GPLv3
        </p>
        <h1 className="text-[clamp(3.5rem,11.5vw,8.5rem)] leading-[0.92] font-bold tracking-[-0.04em]">
          Garde
          <br />
          le meilleur.
        </h1>
        <p className="text-muted mt-8 max-w-[34rem] text-xl leading-relaxed">
          ClipForge garde en mémoire les dernières secondes de ton jeu. Quand il se passe quelque
          chose, appuie sur <Kbd className="text-accent">F8</Kbd>. Le jeu continue, le moment reste.
        </p>
        <div className="mt-9 flex flex-wrap gap-3">
          <Button asChild variant="primary" size="lg">
            <Link href="/telechargement/">
              <Windows /> Télécharger pour Windows
            </Link>
          </Button>
          <Button asChild variant="secondary" size="lg">
            <a href={REPO_URL} target="_blank" rel="noopener noreferrer">
              <Github /> Voir le code
            </a>
          </Button>
        </div>
        <p className="text-muted mt-4 text-sm">
          Version 0.7.0 · Windows x64 · aucune connexion requise pour capturer
        </p>
      </div>
      <div className="mx-auto mt-16 w-[min(72rem,100%-2rem)] sm:w-[min(72rem,100%-3rem)]">
        <ReplayRuler />
      </div>
    </section>
  );
}
