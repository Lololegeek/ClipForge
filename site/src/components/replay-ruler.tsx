"use client";

import NumberFlow from "@number-flow/react";
import { useAutoAnimate } from "@formkit/auto-animate/react";
import { AnimatePresence, motion } from "motion/react";
import { useEffect, useMemo, useState } from "react";
import { useHotkeys } from "react-hotkeys-hook";
import { useInView } from "react-intersection-observer";
import { Pause, Play } from "lucide-react";
import { KeyCap } from "@/components/keycap";
import { Button } from "@/components/ui/button";
import { Slider } from "@/components/ui/slider";
import { saveReplayWithToast } from "@/lib/actions";
import { DEMO_SPEED, useDemo } from "@/lib/store";
import { cn } from "@/lib/utils";

function tickStep(seconds: number) {
  if (seconds <= 30) return 1;
  if (seconds <= 60) return 2;
  if (seconds <= 120) return 5;
  return 10;
}

export function ReplayRuler() {
  const { replaySeconds, buffer, active, flash, lastSave, clips } =
    useDemo();
  const setReplaySeconds = useDemo((s) => s.setReplaySeconds);
  const toggleActive = useDemo((s) => s.toggleActive);
  const tick = useDemo((s) => s.tick);
  const { ref, inView } = useInView({ rootMargin: "100px" });
  const [listRef] = useAutoAnimate<HTMLUListElement>({ duration: 220 });
  const [showSave, setShowSave] = useState(false);

  useHotkeys(
    "f8",
    (e) => {
      e.preventDefault();
      saveReplayWithToast();
    },
    { enableOnFormTags: true },
  );

  useEffect(() => {
    if (!inView) return;
    let raf = 0;
    let last = performance.now();
    const loop = (now: number) => {
      tick(Math.min(0.1, (now - last) / 1000));
      last = now;
      raf = requestAnimationFrame(loop);
    };
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [inView, tick]);

  useEffect(() => {
    let timer: ReturnType<typeof setTimeout>;
    const unsubscribe = useDemo.subscribe((state, prev) => {
      if (state.lastSave === prev.lastSave) return;
      setShowSave(true);
      clearTimeout(timer);
      timer = setTimeout(() => setShowSave(false), 1700);
    });
    return () => {
      unsubscribe();
      clearTimeout(timer);
    };
  }, []);

  const step = tickStep(replaySeconds);
  const ticks = useMemo(
    () => Array.from({ length: Math.floor(replaySeconds / step) + 1 }, (_, i) => i * step),
    [replaySeconds, step],
  );
  const major = step * 5;
  const pct = (buffer / replaySeconds) * 100;
  const demoClips = clips.filter((c) => c.origin === "demo").slice(0, 3);
  const full = buffer >= replaySeconds - 0.01;

  return (
    <div
      ref={ref}
      className="border-line bg-surface relative overflow-hidden rounded-2xl border p-5 shadow-[0_30px_70px_-40px_rgba(0,0,0,0.55)] sm:p-8"
    >
      <AnimatePresence>
        {flash > 0 && (
          <motion.div
            key={flash}
            aria-hidden
            className="pointer-events-none absolute inset-0 z-10 bg-white"
            initial={{ opacity: 0.8 }}
            animate={{ opacity: 0 }}
            transition={{ duration: 0.45 }}
          />
        )}
      </AnimatePresence>

      <div className="flex flex-wrap items-center gap-x-4 gap-y-2">
        <h2 className="text-xl font-semibold tracking-tight">Instant Replay</h2>
        <span
          className={cn(
            "border-line inline-flex items-center gap-2 rounded-full border px-3 py-1 text-[0.8125rem]",
            active ? "text-ink" : "text-muted",
          )}
        >
          <span
            className={cn(
              "size-2 rounded-full",
              active ? "bg-ok [animation:pulse-ring_1.6s_ease-out_infinite]" : "bg-muted",
            )}
          />
          {active ? "Tampon actif" : "Capture en veille"}
        </span>
        <div className="text-accent tnum ml-auto font-mono text-base">
          <NumberFlow value={Math.floor(buffer)} /> / {replaySeconds} s
        </div>
      </div>

      {/* Règle : la durée réellement couverte par le tampon, rien d'autre. */}
      <div
        className="relative mt-6 h-[4.5rem] select-none"
        role="img"
        aria-label={`Le tampon couvre ${Math.floor(buffer)} secondes sur ${replaySeconds}`}
      >
        <div className="bg-line absolute inset-x-0 top-[2.125rem] h-1 rounded-full" />
        <div
          className="bg-accent absolute top-[2.125rem] left-0 h-1 rounded-full"
          style={{ width: `${pct}%` }}
        />
        <div
          className="bg-ink absolute top-[1.4rem] h-[1.1rem] w-0.5 rounded-full transition-opacity"
          style={{ left: `calc(${pct}% - 1px)`, opacity: active ? 1 : 0.3 }}
        />
        <AnimatePresence>
          {showSave && lastSave && (
            <motion.div
              key={lastSave.at}
              className="border-accent bg-accent/20 absolute top-3.5 h-11 rounded-lg border"
              style={{ left: 0, width: `${(lastSave.seconds / replaySeconds) * 100}%` }}
              initial={{ opacity: 0, scaleX: 0.9, originX: 1 }}
              animate={{ opacity: 1, scaleX: 1 }}
              exit={{ opacity: 0 }}
              transition={{ duration: 0.25 }}
            >
              <span className="text-accent absolute -top-6 left-2 font-mono text-xs">
                −{lastSave.seconds} s sauvegardées
              </span>
            </motion.div>
          )}
        </AnimatePresence>
        <div className="absolute inset-x-0 bottom-1 flex items-end justify-between" aria-hidden>
          {ticks.map((t) => (
            <i key={t} className={cn("bg-muted/70 w-px", t % major === 0 ? "h-2.5" : "h-1.5")} />
          ))}
        </div>
      </div>
      <div className="text-muted tnum mt-1 flex justify-between font-mono text-xs">
        <span>−{replaySeconds} s</span>
        <span>maintenant{full && active ? " · tampon plein" : ""}</span>
      </div>

      <div className="mt-8 grid gap-6 lg:grid-cols-[minmax(0,1fr)_auto] lg:items-end">
        <div className="grid gap-5">
          <div className="max-w-md">
            <div className="mb-2 flex items-baseline justify-between text-sm">
              <label htmlFor="duree" className="text-muted">
                Durée du tampon
              </label>
              <span className="text-ink tnum font-mono">{replaySeconds} s</span>
            </div>
            <Slider
              id="duree"
              min={15}
              max={120}
              step={5}
              value={[replaySeconds]}
              onValueChange={([v]) => setReplaySeconds(v)}
              aria-label="Durée du tampon en secondes"
            />
            <p className="text-muted mt-2 text-[0.8125rem]">
              Réglable de 5 s à 1 h dans l’application. 60 s par défaut.
            </p>
          </div>
          <Button variant="secondary" size="sm" className="w-fit" onClick={toggleActive}>
            {active ? <Pause /> : <Play />} {active ? "Arrêter le replay" : "Activer le replay"}
          </Button>
        </div>
        <div className="flex flex-wrap gap-3">
          <KeyCap
            keyName="F8"
            label="Sauvegarder"
            hint="Écrit le contenu du tampon en MP4. Touche modifiable (F1 à F24)."
            onPress={saveReplayWithToast}
            disabled={!active}
          />
        </div>
      </div>

      <ul
        ref={listRef}
        aria-live="polite"
        className="border-line mt-8 grid min-h-[3.25rem] gap-2 border-t pt-5"
      >
        {demoClips.length === 0 ? (
          <li className="text-muted text-sm">
            Aucun clip sauvegardé. Appuie sur F8, ou touche le bouton ci-dessus.
          </li>
        ) : (
          demoClips.map((c) => (
            <li
              key={c.id}
              className="border-line bg-canvas flex max-w-xl items-center gap-3 rounded-lg border px-3.5 py-2.5 text-sm"
            >
              <span className="bg-ok size-2 shrink-0 rounded-full" />
              <span className="min-w-0 flex-1 truncate">{c.name}.mp4</span>
              <span className="text-muted tnum font-mono">{c.seconds} s</span>
            </li>
          ))
        )}
      </ul>
      <p className="text-muted mt-4 text-[0.8125rem]">
        Démo dans le navigateur, accélérée ×{DEMO_SPEED} : rien n’est enregistré. La touche F8
        du clavier fonctionne ici aussi.
      </p>
    </div>
  );
}
