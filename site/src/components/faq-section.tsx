"use client";

import * as Accordion from "@radix-ui/react-accordion";
import { Plus } from "lucide-react";
import Link from "next/link";
import { Section, SectionTitle } from "@/components/sections";

const FAQ = [
  {
    q: "Mes vidéos partent-elles quelque part ?",
    a: "Non. ClipForge fonctionne hors ligne, sans compte ni télémétrie. Clips, captures et bibliothèque restent sur ton disque.",
  },
  {
    q: "Ça marche avec mon jeu et son anti-triche ?",
    a: "Ce n’est pas vérifié sur de vrais jeux. ClipForge utilise des méthodes de capture standard (DXGI, Windows Graphics Capture, capture de jeu OBS) et ne tente jamais de contourner un anti-triche. La présence d’une fenêtre ne garantit pas sa compatibilité.",
  },
  {
    q: "Pourquoi l’enregistrement est en MKV et les replays en MP4 ?",
    a: "Le MKV résiste mieux à une interruption : si le jeu ou le PC plante, ce qui a été écrit reste lisible. Un replay est court et finalisé tout de suite, donc le MP4 suffit. L’éditeur exporte en MP4.",
  },
  {
    q: "Le replay est-il écrit en continu sur mon SSD ?",
    a: "Non. Le tampon vit dans la RAM (512 Mo maximum par défaut, réglable) et rien n’est écrit tant que tu n’appuies pas sur F8. À haut débit, la limite mémoire peut raccourcir la durée conservée.",
  },
  {
    q: "Puis-je le compiler moi-même ?",
    a: "Oui, c’est aujourd’hui le moyen d’obtenir ClipForge. Les scripts PowerShell du dépôt téléchargent les dépendances dans un dossier isolé et produisent un dossier portable.",
  },
  {
    q: "Pourquoi n’y a-t-il pas encore d’installateur ?",
    a: "ClipForge embarque OBS, FFmpeg et Qt, sous GPL ou licences compatibles. Avant de distribuer des binaires, il faut fournir les sources correspondantes de chaque composant : cet inventaire n’est pas terminé.",
  },
];

export function FaqSection() {
  return (
    <Section id="faq">
      <div className="grid gap-12 lg:grid-cols-[minmax(0,5fr)_minmax(0,7fr)] lg:gap-20">
        <div>
          <SectionTitle>
            Questions
            <br />
            fréquentes.
          </SectionTitle>
          <p className="text-muted mt-6 max-w-sm text-lg">
            Le détail de chaque limite est dans la page{" "}
            <Link href="/fonctionnalites/" className="text-accent underline underline-offset-4">
              fonctionnalités
            </Link>
            .
          </p>
        </div>
        <Accordion.Root type="single" collapsible className="border-line border-t">
          {FAQ.map((f) => (
            <Accordion.Item key={f.q} value={f.q} className="border-line border-b">
              <Accordion.Header>
                <Accordion.Trigger className="group hover:text-accent flex w-full cursor-pointer items-center justify-between gap-6 py-5 text-left text-lg font-medium transition-colors">
                  {f.q}
                  <Plus className="text-muted size-5 shrink-0 transition-transform duration-200 group-data-[state=open]:rotate-45" />
                </Accordion.Trigger>
              </Accordion.Header>
              <Accordion.Content className="data-[state=closed]:animate-out data-[state=closed]:fade-out-0 data-[state=open]:animate-in data-[state=open]:fade-in-0 overflow-hidden">
                <p className="text-muted max-w-xl pb-6">{f.a}</p>
              </Accordion.Content>
            </Accordion.Item>
          ))}
        </Accordion.Root>
      </div>
    </Section>
  );
}
