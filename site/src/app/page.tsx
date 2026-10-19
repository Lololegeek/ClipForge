import { AppTour } from "@/components/app-tour";
import { DownloadSection } from "@/components/download-panel";
import { FaqSection } from "@/components/faq-section";
import { Hero } from "@/components/hero";
import { Reveal } from "@/components/reveal";
import {
  JourneySection,
  LimitsSection,
  Section,
  SectionTitle,
  TechSection,
} from "@/components/sections";

export default function Home() {
  return (
    <>
      <Hero />
      <JourneySection />
      <Section id="appli" className="border-line bg-nav border-y">
        <div className="grid items-end gap-6 md:grid-cols-2 md:gap-12">
          <SectionTitle>L’application.</SectionTitle>
          <p className="text-muted max-w-md text-lg">
            Une barre latérale, six pages, l’état de la capture toujours visible. Le thème sombre
            est pensé pour être consulté pendant que le jeu tourne. Explore les captures de chaque écran.
          </p>
        </div>
        <Reveal className="mt-12">
          <AppTour />
        </Reveal>
      </Section>
      <TechSection />
      <LimitsSection />
      <FaqSection />
      <DownloadSection />
    </>
  );
}
