import type { Metadata } from "next";
import { JourneySection } from "@/components/sections";

export const metadata: Metadata = {
  title: "Parcours",
  description: "Du jeu au clip : le parcours d’un replay avec ClipForge.",
};

export default function JourneyPage() {
  return (
    <>
      <h1 className="sr-only">Parcours de replay</h1>
      <JourneySection />
    </>
  );
}
