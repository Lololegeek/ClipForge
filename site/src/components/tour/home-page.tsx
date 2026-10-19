"use client";

import NumberFlow from "@number-flow/react";
import { useDemo } from "@/lib/store";
import { saveReplayWithToast } from "@/lib/actions";
import { AppButton, PageTitle } from "@/components/tour/ui";

export function HomePage({ goto }: { goto: (page: "clips") => void }) {
  const { buffer, replaySeconds, active, clips } = useDemo();
  const toggleActive = useDemo((s) => s.toggleActive);
  const pct = Math.min(100, (buffer / replaySeconds) * 100);

  return (
    <div className="@container grid content-start gap-5">
      <div>
        <PageTitle>Garde le meilleur.</PageTitle>
        <p className="text-muted mt-1 text-[14px]">Le jeu continue. Le moment reste.</p>
      </div>

      <div className="bg-surface rounded-[10px] p-5">
        <div className="flex items-center justify-between">
          <h4 className="text-[20px] font-semibold">Instant Replay</h4>
          <span className="text-accent tnum font-mono text-[16px]">
            <NumberFlow value={Math.floor(buffer)} /> / {replaySeconds} s
          </span>
        </div>
        <div className="relative my-4 h-10">
          <div className="bg-line absolute inset-x-0 top-[18px] h-1 rounded-full" />
          <div
            className="bg-accent absolute top-[18px] h-1 rounded-full"
            style={{ width: `${pct}%` }}
          />
          <div className="absolute inset-x-0 bottom-0 flex items-end justify-between">
            {Array.from({ length: 31 }, (_, i) => (
              <i key={i} className={`bg-muted w-px ${i % 5 === 0 ? "h-2.5" : "h-1"}`} />
            ))}
          </div>
        </div>
        <div className="flex items-center gap-4">
          <p className="text-muted flex-1 text-[12px] leading-relaxed">
            DXGI · écran principal
            <br />
            AMD AMF H.264
          </p>
          <AppButton primary onClick={toggleActive}>
            {active ? "Arrêter" : "Activer le replay"}
          </AppButton>
        </div>
      </div>

      <dl className="flex flex-wrap gap-x-10 gap-y-3">
        {[
          ["Clips", <NumberFlow key="c" value={clips.length} />],
          ["RAM", "412 Mo"],
          ["Capture", "60.0 FPS"],
          ["Stockage libre", "212.4 Go"],
        ].map(([label, value]) => (
          <div key={label as string}>
            <dd className="tnum font-mono text-[19px]">{value}</dd>
            <dt className="text-muted text-[12px]">{label}</dt>
          </div>
        ))}
      </dl>

      <div className="flex items-center justify-between">
        <h4 className="text-[20px] font-semibold">Derniers moments</h4>
        <AppButton onClick={() => goto("clips")}>Voir mes clips</AppButton>
      </div>
      <ul className="grid gap-0.5">
        {clips.slice(0, 3).map((c, i) => (
          <li
            key={c.id}
            className={`text-muted flex items-center gap-4 rounded-[7px] px-3.5 py-[17px] text-[14px] ${i === 0 ? "bg-surface" : ""}`}
          >
            <span className="text-ink min-w-0 flex-1 truncate">{c.name}</span>
            <span className="hidden @md:inline">{c.game}</span>
            <span className="tnum font-mono">{c.seconds} s →</span>
          </li>
        ))}
      </ul>
      <p className="text-muted text-[12px]">
        <button
          type="button"
          className="text-accent cursor-pointer underline underline-offset-2"
          onClick={saveReplayWithToast}
        >
          Sauvegarder un replay
        </button>{" "}
        ajoute un clip à cette liste.
      </p>
    </div>
  );
}
