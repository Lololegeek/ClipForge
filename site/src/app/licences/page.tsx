import type { Metadata } from "next";
import { Markdown } from "@/components/markdown";
import { readRepoDoc } from "@/lib/docs";

export const metadata: Metadata = {
  title: "Licences tierces",
  description: "Inventaire des composants tiers distribués avec ClipForge.",
};

export default async function LicensesPage() {
  const doc = await readRepoDoc("THIRD_PARTY.md");
  return (
    <div className="mx-auto w-[min(60rem,100%-2rem)] py-20 sm:w-[min(60rem,100%-3rem)] sm:py-28">
      <Markdown>{doc}</Markdown>
    </div>
  );
}
