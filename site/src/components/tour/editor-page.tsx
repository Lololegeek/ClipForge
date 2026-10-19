"use client";

import * as Progress from "@radix-ui/react-progress";
import * as ToggleGroup from "@radix-ui/react-toggle-group";
import { Pause, Play } from "lucide-react";
import { useEffect, useRef, useState } from "react";
import { toast } from "sonner";
import { Thumb } from "@/components/tour/thumb";
import { AppButton, PageTitle } from "@/components/tour/ui";
import { Slider } from "@/components/ui/slider";
import { useDemo } from "@/lib/store";
import { cn } from "@/lib/utils";

const SPEEDS = [0.25, 0.5, 1, 2, 4];
type Segment = { from: number; to: number; speed: number };

export function EditorPage({ clipId }: { clipId: string }) {
  const clip = useDemo((s) => s.clips.find((c) => c.id === clipId) ?? s.clips[0]);
  const duration = clip.seconds;
  const [pos, setPos] = useState(0);
  const [playing, setPlaying] = useState(false);
  const [speed, setSpeed] = useState(1);
  const [range, setRange] = useState<[number, number]>([
    Math.round(duration * 0.2),
    Math.round(duration * 0.7),
  ]);
  const [segments, setSegments] = useState<Segment[]>([]);
  const [exporting, setExporting] = useState<number | null>(null);
  const posRef = useRef(0);
  const seek = (v: number) => {
    posRef.current = v;
    setPos(v);
  };

  useEffect(() => {
    if (!playing) return;
    let raf = 0;
    let last = performance.now();
    const loop = (now: number) => {
      const next = posRef.current + ((now - last) / 1000) * speed;
      last = now;
      if (next >= duration) {
        seek(duration);
        setPlaying(false);
        return;
      }
      seek(next);
      raf = requestAnimationFrame(loop);
    };
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [playing, speed, duration]);

  useEffect(() => {
    if (exporting === null) return;
    const t = setTimeout(() => {
      const next = exporting + 4 + Math.random() * 7;
      if (next < 100) {
        setExporting(next);
        return;
      }
      toast.success("Export terminé (démo)", {
        description: "Dans l’application, un nouveau MP4 est écrit. L’original reste intact.",
      });
      setExporting(null);
    }, 110);
    return () => clearTimeout(t);
  }, [exporting]);

  const keep = () => setSegments([{ from: range[0], to: range[1], speed }]);
  const add = () => setSegments((s) => [...s, { from: range[0], to: range[1], speed }]);
  const fmt = (n: number) => n.toFixed(2);

  return (
    <div className="@container grid content-start gap-3.5">
      <div className="flex flex-wrap items-center gap-3">
        <PageTitle>Éditeur</PageTitle>
        <div className="ml-auto flex gap-2">
          <AppButton disabled>Ouvrir un projet</AppButton>
          <AppButton disabled>Enregistrer le projet</AppButton>
        </div>
      </div>
      <p className="text-muted -mt-1 truncate text-[14px]">{clip.name}</p>

      <Thumb tone={clip.tone} className="h-[180px] rounded-[10px] @3xl:h-[200px]">
        <button
          type="button"
          onClick={() => setPlaying((p) => !p)}
          aria-label={playing ? "Pause" : "Lire"}
          className="bg-canvas/70 text-ink absolute top-1/2 left-1/2 grid size-16 -translate-x-1/2 -translate-y-1/2 cursor-pointer place-items-center rounded-full transition-transform hover:scale-105"
        >
          {playing ? <Pause className="size-6" /> : <Play className="size-6 translate-x-0.5" />}
        </button>
      </Thumb>

      <div className="flex flex-wrap items-center gap-x-4 gap-y-2">
        <AppButton onClick={() => setPlaying((p) => !p)} className="h-9">
          {playing ? "Pause" : "Lire"}
        </AppButton>
        <span className="text-muted tnum w-28 font-mono text-[13px]">
          {pos.toFixed(1)} / {duration.toFixed(1)} s
        </span>
        <Slider
          className="min-w-32 flex-1"
          min={0}
          max={duration}
          step={0.1}
          value={[pos]}
          onValueChange={([v]) => seek(v)}
          aria-label="Position de lecture"
        />
        <ToggleGroup.Root
          type="single"
          value={String(speed)}
          onValueChange={(v) => v && setSpeed(Number(v))}
          aria-label="Vitesse de lecture"
          className="bg-surface flex rounded-lg p-0.5"
        >
          {SPEEDS.map((s) => (
            <ToggleGroup.Item
              key={s}
              value={String(s)}
              className="text-muted hover:text-ink data-[state=on]:bg-accent-surface data-[state=on]:text-accent h-8 cursor-pointer rounded-lg px-2.5 font-mono text-[12px] transition-colors"
            >
              {s}×
            </ToggleGroup.Item>
          ))}
        </ToggleGroup.Root>
      </div>

      <div className="bg-surface rounded-lg px-3 py-2.5">
        <Slider
          min={0}
          max={duration}
          step={0.1}
          minStepsBetweenThumbs={5}
          value={range}
          onValueChange={(v) => setRange([v[0], v[1]])}
          aria-label="Début et fin du passage"
        />
      </div>

      <div className="flex flex-wrap items-center gap-x-3 gap-y-2">
        <AppButton
          className="h-9"
          onClick={() => pos < range[1] && setRange([Math.round(pos * 10) / 10, range[1]])}
        >
          Entrée ici
        </AppButton>
        <span className="text-accent tnum font-mono text-[14px]">{fmt(range[0])} s</span>
        <AppButton
          className="h-9"
          onClick={() => pos > range[0] && setRange([range[0], Math.round(pos * 10) / 10])}
        >
          Sortie ici
        </AppButton>
        <span className="text-accent tnum font-mono text-[14px]">{fmt(range[1])} s</span>
        <span className="flex-1" />
        <AppButton className="h-9" onClick={add}>
          Ajouter
        </AppButton>
        <AppButton primary className="h-9" onClick={keep}>
          Garder ce passage
        </AppButton>
      </div>

      {segments.length > 0 && (
        <ul className="grid gap-1.5" aria-label="Passages exportés dans cet ordre">
          {segments.map((s, i) => (
            <li
              key={i}
              className="bg-surface flex items-center justify-between rounded-lg px-3.5 py-2 text-[13px]"
            >
              <span className="tnum font-mono">
                {i + 1}&nbsp;&nbsp; {fmt(s.from)} → {fmt(s.to)} s · {s.speed}×
              </span>
              <button
                type="button"
                className="text-muted hover:text-ink cursor-pointer"
                onClick={() => setSegments((all) => all.filter((_, j) => j !== i))}
              >
                Retirer
              </button>
            </li>
          ))}
        </ul>
      )}

      <div className="flex flex-wrap items-center gap-x-4 gap-y-2">
        <p className="text-muted min-w-0 flex-1 text-[12px]">
          Montage non destructif · export précis avec réencodage · la vidéo originale reste intacte.
        </p>
        {exporting !== null && (
          <Progress.Root
            value={exporting}
            className="bg-line h-1.5 w-28 overflow-hidden rounded-full"
            aria-label="Progression de l’export"
          >
            <Progress.Indicator
              className="bg-accent h-full transition-transform"
              style={{ transform: `translateX(-${100 - exporting}%)` }}
            />
          </Progress.Root>
        )}
        <AppButton
          primary
          disabled={exporting !== null}
          onClick={() => setExporting(0)}
          className={cn("min-w-40", exporting !== null && "opacity-100")}
        >
          {exporting !== null ? `Export ${Math.round(exporting)} %` : "Exporter en MP4"}
        </AppButton>
      </div>
    </div>
  );
}
