"use client";

import { Play } from "lucide-react";
import { useState } from "react";

const VIDEO_ID = "q1wql-EXWp4";
const TITLE = "ClipForge : F8 et ton meilleur moment est sauvegardé";
const basePath = process.env.NEXT_PUBLIC_BASE_PATH ?? "";

/**
 * The YouTube Short of the channel. Nothing is loaded from YouTube until the visitor clicks:
 * the preview image is served by the site itself, then the privacy-enhanced player replaces it.
 */
export function DemoVideo({ className = "" }: { className?: string }) {
  const [playing, setPlaying] = useState(false);
  return (
    <figure className={className}>
      <div className="border-line bg-nav relative aspect-[9/16] w-full overflow-hidden rounded-2xl border shadow-2xl">
        {playing ? (
          <iframe
            className="absolute inset-0 size-full"
            src={`https://www.youtube-nocookie.com/embed/${VIDEO_ID}?autoplay=1&rel=0&playsinline=1`}
            title={TITLE}
            allow="autoplay; encrypted-media; picture-in-picture; fullscreen"
            referrerPolicy="strict-origin-when-cross-origin"
            allowFullScreen
          />
        ) : (
          <button
            type="button"
            onClick={() => setPlaying(true)}
            className="group absolute inset-0 cursor-pointer"
            aria-label={`Lire la vidéo : ${TITLE}`}
          >
            {/* eslint-disable-next-line @next/next/no-img-element */}
            <img
              src={`${basePath}/video-demo.jpg`}
              alt=""
              className="absolute inset-0 size-full object-cover transition-transform duration-500 group-hover:scale-[1.03]"
            />
            <span className="absolute inset-0 bg-gradient-to-t from-black/70 via-black/10 to-transparent" />
            <span className="bg-accent text-accent-ink absolute top-1/2 left-1/2 grid size-16 -translate-x-1/2 -translate-y-1/2 place-items-center rounded-full shadow-lg transition-transform group-hover:scale-110">
              <Play className="size-7 translate-x-0.5 fill-current" />
            </span>
            <span className="absolute inset-x-4 bottom-4 text-left text-sm font-semibold text-white">
              {TITLE}
            </span>
          </button>
        )}
      </div>
      <figcaption className="text-muted mt-3 text-center text-xs">
        {playing
          ? "Lecture via YouTube (mode confidentialité renforcée)."
          : "La vidéo se charge depuis YouTube au clic."}
      </figcaption>
    </figure>
  );
}
