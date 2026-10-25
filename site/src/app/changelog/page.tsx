import type { Metadata } from "next";
import Link from "next/link";
import { ArrowUpRight } from "lucide-react";
import { RELEASES_URL } from "@/lib/site";

export const metadata: Metadata = {
  title: "Changelog",
  description: "Nouveautés et changements de chaque version de ClipForge.",
};

const releases = [
  {
    version: "0.6.0",
    date: "25 octobre 2026",
    summary: "Le replay est actif dès le lancement, et clips, éditeur et miniatures s’affichent plus vite.",
    changes: [
      "Instant Replay démarre automatiquement dès l’ouverture de ClipForge, redémarre après un changement de réglages et retente en cas d’erreur. Une option permet de le désactiver.",
      "Les miniatures sont générées en parallèle, avec priorité aux clips que vous venez de sauvegarder.",
      "La grille de clips réutilise ses éléments et met les miniatures en cache pour défiler plus fluidement.",
      "L’éditeur affiche la miniature instantanément avant la vidéo; le déplacement sur la barre de lecture est lissé côté audio.",
      "Nouveaux raccourcis de l’éditeur : image par image avec , et . et coupure du son avec M.",
      "Un avertissement apparaît quand l’espace disque passe sous 2 Go.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.6.0",
  },
  {
    version: "0.5.0",
    date: "20 octobre 2026",
    summary: "Une bibliothèque plus rapide, un montage plus pratique et une capture plus résiliente.",
    changes: [
      "La bibliothèque évite de réanalyser les clips inchangés, réutilise les miniatures et met à jour les lignes sans recharger toute la liste.",
      "La recherche traite % et _ comme des caractères normaux. Les raccourcis Ctrl+F et Échap facilitent son utilisation.",
      "L’éditeur gagne les repères en mm:ss, l’aperçu de plage, une barre de découpe retravaillée et des raccourcis clavier pour lire, découper et annuler.",
      "Si l’encodeur matériel ne démarre plus ou échoue, ClipForge peut reprendre en H.264 logiciel. Le tampon repart à zéro et l’enregistrement manuel en cours s’arrête.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.5.0",
  },
  {
    version: "0.4.0",
    date: "19 octobre 2026",
    summary: "Les actions de clip sont recentrées et plus faciles à lire.",
    changes: [
      "Les points du menu d’actions sont maintenant centrés dans leur bouton.",
      "La version de l’application et de son installateur passe à 0.4.0.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.4.0",
  },
  {
    version: "0.3.0",
    date: "19 octobre 2026",
    summary: "ClipForge se concentre sur la sauvegarde des replays.",
    changes: [
      "Une seule touche configurable sauvegarde le replay (F8 par défaut, F1 à F24 disponibles).",
      "Les commandes de capture d’écran et d’enregistrement manuel quittent l’interface; F9 et F10 ne sont plus réservées.",
      "Les téléchargements de mise à jour sont écrits dans un fichier temporaire et vérifiés avec le SHA-256 de GitHub avant installation.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.3.0",
  },
  {
    version: "0.2.0",
    date: "19 octobre 2026",
    summary: "Retour de capture plus clair et raccourcis mieux protégés.",
    changes: [
      "Notifications facultatives pour les clips, les enregistrements et les erreurs.",
      "Les appuis répétés sur les raccourcis sont limités pour éviter les sauvegardes en double.",
      "Ajout du démarrage à l’ouverture de session et du téléchargement facultatif des mises à jour.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.2.0",
  },
  {
    version: "0.1.1",
    date: "19 octobre 2026",
    summary: "Les icônes Windows reprennent le logo du site.",
    changes: [
      "Correction de l’icône de l’application, de la fenêtre, de la barre des tâches et des raccourcis Windows.",
      "Les clips et les réglages existants sont préservés pendant la mise à jour.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.1.1",
  },
  {
    version: "0.1.0",
    date: "19 octobre 2026",
    summary: "Première version publique pour Windows x64.",
    changes: [
      "Replay hors ligne, bibliothèque locale et montage MP4 non destructif.",
      "Installateur par utilisateur sans droits administrateur, compte ou activation en ligne.",
      "Le logo du site est partagé par l’application, la barre des tâches et la zone de notification.",
    ],
    url: "https://github.com/Lololegeek/ClipForge/releases/tag/v0.1.0",
  },
];

export default function ChangelogPage() {
  return (
    <div className="mx-auto w-[min(60rem,100%-2rem)] py-16 sm:w-[min(60rem,100%-3rem)] sm:py-24">
      <header className="mb-14 max-w-2xl">
        <p className="text-accent mb-4 font-mono text-sm tracking-widest uppercase">
          Historique des versions
        </p>
        <h1 className="text-5xl font-semibold tracking-tight sm:text-6xl">Changelog.</h1>
        <p className="text-muted mt-5 text-lg">
          Les changements de ClipForge, version après version. Pour les fichiers et les sommes de
          contrôle, ouvre la release correspondante sur GitHub.
        </p>
      </header>

      <div className="border-line ml-2 border-l">
        {releases.map((release, index) => (
          <article
            key={release.version}
            className={`relative pb-12 pl-8 sm:pl-10 ${index === releases.length - 1 ? "pb-0" : ""}`}
          >
            <span className="bg-accent absolute top-1 left-[-5px] size-2.5 rounded-full ring-4 ring-[var(--canvas)]" />
            <div className="flex flex-wrap items-center gap-x-4 gap-y-2">
              <h2 className="text-ink font-mono text-2xl font-semibold">v{release.version}</h2>
              <span className="text-muted font-mono text-xs tracking-wider uppercase">
                {release.date}
              </span>
              {index === 0 && (
                <span className="bg-accent-surface text-accent rounded-full px-2.5 py-1 font-mono text-xs">
                  Dernière version
                </span>
              )}
            </div>
            <p className="text-ink mt-3 text-lg">{release.summary}</p>
            <ul className="text-muted mt-5 grid gap-3">
              {release.changes.map((change) => (
                <li key={change} className="relative pl-5 before:absolute before:top-[0.65em] before:left-0 before:size-1.5 before:rounded-full before:bg-current">
                  {change}
                </li>
              ))}
            </ul>
            <Link
              href={release.url}
              className="text-accent hover:text-ink mt-5 inline-flex items-center gap-1.5 text-sm transition-colors"
              target="_blank"
              rel="noreferrer"
            >
              Notes et téléchargements <ArrowUpRight className="size-4" />
            </Link>
          </article>
        ))}
      </div>

      <p className="text-muted border-line mt-16 border-t pt-6 text-sm">
        <Link className="text-accent hover:text-ink transition-colors" href={RELEASES_URL} target="_blank" rel="noreferrer">
          Toutes les releases GitHub <ArrowUpRight className="ml-1 inline size-3.5" />
        </Link>
      </p>
    </div>
  );
}
