import type { Metadata } from "next";
import { Markdown } from "@/components/markdown";
import { readRepoDoc } from "@/lib/docs";

export const metadata: Metadata = {
  title: "Fonctionnalités",
  description:
    "Périmètre exact de ClipForge 0.1 : ce qui fonctionne, ce qui est limité, ce qui manque.",
};

export default async function FeaturesPage() {
  const features = await readRepoDoc("docs/FEATURES.md");
  const capture = await readRepoDoc("docs/CAPTURE.md");
  return (
    <div className="mx-auto w-[min(60rem,100%-2rem)] py-20 sm:w-[min(60rem,100%-3rem)] sm:py-28">
      <p className="text-accent mb-4 font-mono text-sm tracking-widest uppercase">Version 0.1</p>
      <Markdown>{features}</Markdown>
      <div className="border-line mt-20 border-t pt-16">
        <Markdown>{capture}</Markdown>
      </div>
      <p className="text-muted mt-16 text-sm">
        Cette page est générée à partir des fichiers <code>docs/FEATURES.md</code> et{" "}
        <code>docs/CAPTURE.md</code> du dépôt.
      </p>
    </div>
  );
}
