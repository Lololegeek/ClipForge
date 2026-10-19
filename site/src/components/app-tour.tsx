"use client";

import * as Tabs from "@radix-ui/react-tabs";
import Image from "next/image";

const SCREENS = [
  { id: "home", label: "Accueil", description: "Accueil de ClipForge et derniers clips importés." },
  { id: "clips", label: "Mes clips", description: "Bibliothèque de vidéos importées dans ClipForge." },
  { id: "record", label: "Enregistrer", description: "Réglages d’enregistrement et de replay de l’application Windows." },
  { id: "editor", label: "Éditeur", description: "Éditeur ClipForge lisant un extrait de Big Buck Bunny." },
  { id: "settings", label: "Paramètres", description: "Paramètres de capture, audio et raccourcis de ClipForge." },
] as const;

export function AppTour() {
  const basePath = process.env.NEXT_PUBLIC_BASE_PATH ?? "";
  return (
    <Tabs.Root defaultValue="home">
      <Tabs.List aria-label="Captures de l’application Windows" className="mb-6 flex flex-wrap gap-2">
        {SCREENS.map((screen) => (
          <Tabs.Trigger
            key={screen.id}
            value={screen.id}
            className="text-muted hover:text-ink data-[state=active]:text-accent data-[state=active]:bg-accent-surface cursor-pointer rounded-lg px-4 py-2 text-[0.9375rem] font-medium transition-colors focus-visible:outline-2 focus-visible:outline-offset-4"
          >
            {screen.label}
          </Tabs.Trigger>
        ))}
      </Tabs.List>
      {SCREENS.map((screen) => (
        <Tabs.Content key={screen.id} value={screen.id}>
          <a
            href={`${basePath}/screenshots/${screen.id}.png`}
            target="_blank"
            rel="noopener noreferrer"
            aria-label={`Agrandir : ${screen.label}`}
            className="border-line block overflow-hidden rounded-xl border focus-visible:outline-2 focus-visible:outline-offset-4"
          >
            <Image
              src={`${basePath}/screenshots/${screen.id}.png`}
              alt={screen.description}
              width={1320}
              height={840}
              className="h-auto w-full"
              priority={screen.id === "home"}
            />
          </a>
        </Tabs.Content>
      ))}
      <p className="text-muted mt-4 text-sm">
        Captures réelles de ClipForge sous Windows. Vidéo importée :{" "}
        <a className="underline underline-offset-4" href="https://peach.blender.org/" target="_blank" rel="noopener noreferrer">
          Big Buck Bunny, © Blender Foundation
        </a>{" "}
        (<a className="underline underline-offset-4" href="https://creativecommons.org/licenses/by/3.0/" target="_blank" rel="noopener noreferrer">CC BY 3.0</a>).
        Clique sur une image pour l’agrandir.
      </p>
    </Tabs.Root>
  );
}
