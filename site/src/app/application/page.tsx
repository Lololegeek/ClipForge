import type { Metadata } from "next";
import { AppTour } from "@/components/app-tour";

export const metadata: Metadata = {
  title: "L’application",
  description: "Découvrir les pages et les captures réelles de ClipForge sous Windows.",
};

export default function ApplicationPage() {
  return (
    <section className="border-line bg-nav border-y py-16 sm:py-24">
      <div className="mx-auto w-[min(72rem,100%-2rem)] sm:w-[min(72rem,100%-3rem)]">
        <div className="grid items-end gap-6 md:grid-cols-2 md:gap-12">
          <h1 className="text-[clamp(2.5rem,6vw,4.5rem)] leading-none font-bold tracking-[-0.035em]">
            L’application.
          </h1>
          <p className="text-muted max-w-md text-lg">
            Une barre latérale, six pages, l’état de la capture toujours visible. Le thème sombre
            est pensé pour être consulté pendant que le jeu tourne. Explore les captures réelles de
            chaque écran.
          </p>
        </div>
        <div className="mt-12">
          <AppTour />
        </div>
      </div>
    </section>
  );
}
