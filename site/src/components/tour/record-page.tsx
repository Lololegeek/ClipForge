"use client";

import NumberFlow from "@number-flow/react";
import { useInView } from "react-intersection-observer";
import { AppButton, PageTitle } from "@/components/tour/ui";
import { screenshotWithToast, toggleRecordingWithToast } from "@/lib/actions";
import { useDemo } from "@/lib/store";

const METRICS: { label: string; value: number; digits?: number; suffix?: string }[] = [
  { label: "FPS effectifs", value: 59.97, digits: 2 },
  { label: "Images en retard", value: 0 },
  { label: "Rendu moyen", value: 0.84, digits: 2, suffix: " ms" },
  { label: "Dernière sauvegarde", value: 186, suffix: " ms" },
  { label: "Mémoire du processus", value: 412, suffix: " Mo" },
  { label: "CPU (tous les cœurs)", value: 2.3, digits: 1, suffix: " %" },
];

export function RecordPage() {
  const { active, recording, replaySeconds } = useDemo();
  const toggleActive = useDemo((s) => s.toggleActive);
  const { ref, inView } = useInView({ triggerOnce: true });

  return (
    <div ref={ref} className="@container grid content-start gap-5">
      <div>
        <PageTitle>Enregistrer</PageTitle>
        <p className="text-muted mt-1 max-w-xl text-[14px]">
          Le replay garde les dernières secondes. L’enregistrement conserve toute la session,
          jusqu’à son arrêt.
        </p>
      </div>
      <div className="bg-surface rounded-[10px] p-5">
        <p className="text-[18px] font-semibold">DXGI · écran principal</p>
        <p className="text-muted mt-1 text-[14px]">AMD AMF H.264</p>
        <p className="text-muted mt-1.5 font-mono text-[13px]">
          1920 × 1080 · 60 FPS · H264 · {replaySeconds} secondes
        </p>
        <div className="mt-5 flex flex-wrap gap-2.5">
          <AppButton onClick={toggleActive}>
            {active ? "Arrêter la capture" : "Activer le replay"}
          </AppButton>
          <AppButton primary onClick={toggleRecordingWithToast}>
            {recording ? "Terminer l’enregistrement" : "Enregistrer · F9"}
          </AppButton>
          <AppButton onClick={screenshotWithToast}>Capture écran · F10</AppButton>
        </div>
      </div>
      <dl className="grid grid-cols-2 gap-x-8 gap-y-5 @3xl:grid-cols-3">
        {METRICS.map((m) => (
          <div key={m.label}>
            <dt className="text-muted text-[12px]">{m.label}</dt>
            <dd className="tnum font-mono text-[22px]">
              <NumberFlow
                value={inView ? m.value : 0}
                format={{
                  minimumFractionDigits: m.digits ?? 0,
                  maximumFractionDigits: m.digits ?? 0,
                }}
                suffix={m.suffix}
              />
            </dd>
          </div>
        ))}
      </dl>
      <p className="text-muted max-w-2xl text-[12px] leading-relaxed">
        La charge GPU et la taille exacte des paquets du tampon ne sont pas exposées ici. La durée
        affichée indique le temps écoulé, plafonné au réglage. Valeurs d’exemple.
      </p>
    </div>
  );
}
