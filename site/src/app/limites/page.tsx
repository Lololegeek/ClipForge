import type { Metadata } from "next";
import { LimitsSection } from "@/components/sections";

export const metadata: Metadata = {
  title: "Limites",
  description: "Ce qui est testé, ce qui reste à vérifier et les limites connues de ClipForge.",
};

export default function LimitsPage() {
  return (
    <>
      <h1 className="sr-only">Limites et état de validation</h1>
      <LimitsSection />
    </>
  );
}
