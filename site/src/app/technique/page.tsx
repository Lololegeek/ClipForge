import type { Metadata } from "next";
import { TechSection } from "@/components/sections";

export const metadata: Metadata = {
  title: "Technique",
  description: "Fonctionnement local et technologies de capture, lecture et export de ClipForge.",
};

export default function TechnicalPage() {
  return (
    <>
      <h1 className="sr-only">Fonctionnement technique</h1>
      <TechSection />
    </>
  );
}
