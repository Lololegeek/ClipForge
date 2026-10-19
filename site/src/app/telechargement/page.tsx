import type { Metadata } from "next";
import { DownloadPanel } from "@/components/download-panel";
import { Markdown } from "@/components/markdown";
import { readRepoDoc } from "@/lib/docs";

export const metadata: Metadata = {
  title: "Télécharger",
  description: "Télécharger ClipForge ou le compiler depuis les sources.",
};

export default async function DownloadPage() {
  const building = await readRepoDoc("docs/BUILDING.md");
  return (
    <div className="mx-auto w-[min(60rem,100%-2rem)] py-20 sm:w-[min(60rem,100%-3rem)] sm:py-28">
      <h1 className="mb-12 text-[clamp(2.5rem,7vw,4.5rem)] leading-none font-bold tracking-[-0.035em]">
        Télécharger.
      </h1>
      <DownloadPanel />
      <div className="border-line mt-20 border-t pt-14">
        <Markdown>{building}</Markdown>
      </div>
    </div>
  );
}
