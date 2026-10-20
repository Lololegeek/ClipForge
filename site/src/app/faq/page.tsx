import type { Metadata } from "next";
import { FaqSection } from "@/components/faq-section";

export const metadata: Metadata = {
  title: "Questions",
  description: "Réponses aux questions fréquentes sur ClipForge.",
};

export default function QuestionsPage() {
  return (
    <>
      <h1 className="sr-only">Questions fréquentes</h1>
      <FaqSection />
    </>
  );
}
