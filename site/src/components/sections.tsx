"use client";

import { Reveal } from "@/components/reveal";
import { Tooltip } from "@/components/ui/tooltip";
import { cn } from "@/lib/utils";

export function Section({
  id,
  className,
  children,
}: {
  id?: string;
  className?: string;
  children: React.ReactNode;
}) {
  return (
    <section id={id} className={cn("scroll-mt-20 py-24 sm:py-32", className)}>
      <div className="mx-auto w-[min(72rem,100%-2rem)] sm:w-[min(72rem,100%-3rem)]">{children}</div>
    </section>
  );
}

export function SectionTitle({ children }: { children: React.ReactNode }) {
  return (
    <h2 className="text-[clamp(2.25rem,5.5vw,4rem)] leading-[1.02] font-bold tracking-[-0.035em]">
      {children}
    </h2>
  );
}

const STEPS = [
  {
    where: "Paramètres",
    title: "Choisir ce qu’on capture",
    body: "Un écran, une fenêtre ou un jeu. Applique les réglages : encodeur, durée du tampon, audio.",
  },
  {
    where: "Accueil",
    title: "Activer Instant Replay",
    body: "Le tampon se remplit en mémoire. La durée réellement couverte est affichée, jamais une estimation flatteuse.",
  },
  {
    where: "En jeu",
    title: "Appuyer sur F8",
    body: "Quand il se passe quelque chose. Le jeu continue, le clip est écrit en MP4 sans interrompre la capture.",
  },
  {
    where: "Mes clips",
    title: "Retrouver, couper, exporter",
    body: "Bibliothèque locale, éditeur non destructif, export MP4. La vidéo originale n’est jamais touchée.",
  },
];

export function JourneySection() {
  return (
    <Section id="parcours">
      <div className="grid gap-12 lg:grid-cols-[minmax(0,5fr)_minmax(0,7fr)] lg:gap-20">
        <div className="lg:sticky lg:top-28 lg:self-start">
          <SectionTitle>
            Du jeu
            <br />
            au clip.
          </SectionTitle>
          <p className="text-muted mt-6 max-w-sm text-lg">
            Quatre gestes, dans cet ordre. Le reste du temps, ClipForge reste dans la zone de
            notification.
          </p>
        </div>
        <ol className="before:bg-line relative grid gap-12 before:absolute before:top-2 before:bottom-2 before:left-[0.95rem] before:w-px">
          {STEPS.map((s, i) => (
            <li key={s.title}>
              <Reveal delay={i * 0.05} className="relative grid grid-cols-[2rem_1fr] gap-6">
                <span className="border-line bg-canvas text-accent relative z-10 grid size-8 place-items-center rounded-full border font-mono text-sm">
                  {i + 1}
                </span>
                <div>
                  <p className="text-muted font-mono text-xs tracking-widest uppercase">
                    {s.where}
                  </p>
                  <h3 className="mt-1 text-2xl font-semibold tracking-tight">{s.title}</h3>
                  <p className="text-muted mt-2 max-w-lg">{s.body}</p>
                </div>
              </Reveal>
            </li>
          ))}
        </ol>
      </div>
    </Section>
  );
}

const ENCODERS = [
  { name: "AMD AMF", status: "ok", note: "H.264 testé sur la machine de développement." },
  { name: "x264", status: "ok", note: "Repli logiciel H.264, testé." },
  {
    name: "NVENC",
    status: "todo",
    note: "Détecté et sélectionnable, pas encore testé sur du matériel NVIDIA.",
  },
  { name: "Intel QSV", status: "todo", note: "Détecté et sélectionnable, pas encore testé." },
  {
    name: "HEVC / AV1",
    status: "todo",
    note: "Disponibles avec un encodeur matériel compatible, non testés.",
  },
] as const;

const SPECS = [
  [
    "Capture",
    "Écran via DXGI, fenêtre via Windows Graphics Capture, ou source jeu OBS standard. Le tampon de replay est compressé en RAM par libobs. Aucun contournement d’anti-triche.",
  ],
  [
    "Audio",
    "Son du système ou d’un processus (WASAPI), micro au choix (désactivé par défaut), gain et sourdine par source. Une piste AAC mixée.",
  ],
  [
    "Bibliothèque",
    "SQLite local : recherche, tri, favoris, tags, renommage, corbeille, import. Un fichier déplacé est signalé, jamais perdu en silence.",
  ],
  [
    "Montage",
    "Plusieurs passages d’une même source, vitesse, gain, annuler/rétablir, projets enregistrés. Export MP4 H.264 ou HEVC, toujours réencodé.",
  ],
  [
    "Jeux",
    "Tu attribues un profil à un exécutable : la capture démarre et s’arrête avec lui. Un processus inconnu n’est jamais pris pour un jeu.",
  ],
] as const;

export function TechSection() {
  return (
    <Section id="technique">
      <SectionTitle>
        Natif, local,
        <br />
        sans détour.
      </SectionTitle>
      <div className="border-line mt-14 border-t">
        <div className="border-line grid gap-4 border-b py-7 md:grid-cols-[12rem_1fr] md:gap-10">
          <p className="text-accent pt-1 font-mono text-sm tracking-wider uppercase">Encodage</p>
          <div>
            <p className="text-muted max-w-2xl">
              Encodeurs matériels détectés au lancement, repli sur x264. Survole une puce pour voir
              ce qui a réellement été testé.
            </p>
            <ul className="mt-4 flex flex-wrap gap-2">
              {ENCODERS.map((e) => (
                <li key={e.name}>
                  <Tooltip content={e.note}>
                    <button
                      type="button"
                      className="border-line bg-surface inline-flex cursor-help items-center gap-2 rounded-full border px-3.5 py-1.5 text-sm"
                    >
                      <span
                        className={cn(
                          "size-2 rounded-full",
                          e.status === "ok" ? "bg-ok" : "bg-warn",
                        )}
                      />
                      {e.name}
                      <span className="sr-only">
                        {e.status === "ok" ? " (testé)" : " (non testé)"}
                      </span>
                    </button>
                  </Tooltip>
                </li>
              ))}
            </ul>
            <p className="text-muted mt-3 flex gap-4 text-[0.8125rem]">
              <span className="inline-flex items-center gap-1.5">
                <i className="bg-ok size-2 rounded-full" /> testé
              </span>
              <span className="inline-flex items-center gap-1.5">
                <i className="bg-warn size-2 rounded-full" /> pas encore testé
              </span>
            </p>
          </div>
        </div>
        {SPECS.map(([k, v], i) => (
          <Reveal key={k} delay={i * 0.03}>
            <div className="border-line hover:bg-surface/50 grid gap-2 border-b py-7 transition-colors md:grid-cols-[12rem_1fr] md:gap-10">
              <p className="text-accent pt-1 font-mono text-sm tracking-wider uppercase">{k}</p>
              <p className="text-muted max-w-2xl">{v}</p>
            </div>
          </Reveal>
        ))}
      </div>
      <p className="text-muted mt-7 font-mono text-sm">
        C++20 · Qt Quick · libobs · FFmpeg · SQLite
      </p>
    </Section>
  );
}

export function LimitsSection() {
  return (
    <Section id="limites" className="border-line bg-nav border-y">
      <SectionTitle>
        Ce qui n’est pas
        <br />
        encore là.
      </SectionTitle>
      <p className="text-muted mt-6 max-w-xl text-lg">
        ClipForge en est à la version 0.4. Voilà, sans arrondir, ce qui est vérifié et ce qui ne
        l’est pas.
      </p>
      <div className="mt-14 grid gap-12 lg:grid-cols-[5fr_7fr] lg:gap-20">
        <div>
          <h3 className="mb-5 text-xl font-semibold">Vérifié sur la machine de test</h3>
          <ul className="grid gap-3.5">
            {[
              "Capture écran et replay avec AMF H.264 et x264",
              "Cohérence audio/vidéo des fichiers exportés",
              "Bibliothèque, montage, export et sauvegarde atomique des réglages",
            ].map((t) => (
              <li
                key={t}
                className="text-muted before:bg-ok relative pl-6 before:absolute before:top-[0.7em] before:left-0 before:h-0.5 before:w-2.5"
              >
                {t}
              </li>
            ))}
          </ul>
        </div>
        <div>
          <h3 className="mb-5 text-xl font-semibold">Pas encore testé, ou absent</h3>
          <ul className="grid gap-3.5">
            {[
              "NVENC, QSV, HEVC et AV1 sur d’autres cartes graphiques",
              "Comportement avec de vrais jeux et leurs anti-triche",
              "Overlay de notifications : plein écran exclusif non vérifié",
              "Exports toujours réencodés, sans texte, sous-titres ni transitions",
              "Replay uniquement en RAM, pas sur disque",
              "Windows 10 et machine vierge : vérifications de publication à faire",
            ].map((t) => (
              <li
                key={t}
                className="text-muted before:bg-warn relative pl-6 before:absolute before:top-[0.7em] before:left-0 before:h-0.5 before:w-2.5"
              >
                {t}
              </li>
            ))}
          </ul>
        </div>
      </div>
    </Section>
  );
}
