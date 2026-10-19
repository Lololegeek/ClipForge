"use client";

import * as Tabs from "@radix-ui/react-tabs";
import { motion } from "motion/react";
import { Clapperboard, CircleDot, Gamepad2, House, Pencil, Settings } from "lucide-react";
import { useState } from "react";
import useMeasure from "react-use-measure";
import { ClipsPage } from "@/components/tour/clips-page";
import { EditorPage } from "@/components/tour/editor-page";
import { HomePage } from "@/components/tour/home-page";
import { RecordPage } from "@/components/tour/record-page";
import { Logo } from "@/components/logo";
import { saveReplayWithToast } from "@/lib/actions";
import { useDemo } from "@/lib/store";
import { cn } from "@/lib/utils";

type PageId = "home" | "clips" | "editor" | "record";

const NAV: {
  id: PageId | null;
  label: string;
  icon: React.ComponentType<{ className?: string }>;
}[] = [
  { id: "home", label: "Accueil", icon: House },
  { id: "clips", label: "Mes clips", icon: Clapperboard },
  { id: "record", label: "Enregistrer", icon: CircleDot },
  { id: "editor", label: "Éditeur", icon: Pencil },
  { id: null, label: "Jeux", icon: Gamepad2 },
  { id: null, label: "Paramètres", icon: Settings },
];

const TABS: { id: PageId; label: string }[] = [
  { id: "home", label: "Accueil" },
  { id: "clips", label: "Mes clips" },
  { id: "editor", label: "Éditeur" },
  { id: "record", label: "Enregistrer" },
];

const W = 1180;
const H = 740;

function StatusBar({ compact }: { compact: boolean }) {
  const active = useDemo((s) => s.active);
  return (
    <div
      className={cn(
        "border-line flex shrink-0 items-center gap-2.5 border-b",
        compact ? "h-14 px-4" : "h-16 px-[30px]",
      )}
    >
      <span className={cn("size-[7px] rounded-full", active ? "bg-ok" : "bg-muted")} />
      <span className="text-[13px]">{active ? "Instant Replay actif" : "Capture en veille"}</span>
      <span className="text-muted ml-auto hidden text-[12px] sm:inline">
        Aucun profil de jeu actif
      </span>
      <button
        type="button"
        disabled={!active}
        onClick={saveReplayWithToast}
        className="bg-accent text-accent-ink ml-auto h-9 cursor-pointer rounded-lg px-4 text-[13px] font-semibold transition hover:brightness-110 disabled:cursor-default disabled:opacity-45 sm:ml-0"
      >
        Sauvegarder · F8
      </button>
    </div>
  );
}

function Pages({
  compact,
  setPage,
  clipId,
  openClip,
}: {
  compact: boolean;
  setPage: (p: PageId) => void;
  clipId: string;
  openClip: (id: string) => void;
}) {
  return (
    <div className={cn("min-h-0 flex-1", compact ? "p-4" : "overflow-hidden p-[30px]")}>
      <Tabs.Content value="home">
        <HomePage goto={() => setPage("clips")} />
      </Tabs.Content>
      <Tabs.Content value="clips">
        <ClipsPage onOpen={openClip} />
      </Tabs.Content>
      <Tabs.Content value="editor">
        <EditorPage key={clipId} clipId={clipId} />
      </Tabs.Content>
      <Tabs.Content value="record">
        <RecordPage />
      </Tabs.Content>
    </div>
  );
}

export function AppTour() {
  const [page, setPage] = useState<PageId>("home");
  const [clipId, setClipId] = useState<string>(() => useDemo.getState().clips[0].id);
  const [ref, { width }] = useMeasure();
  const scale = width / W;
  const compact = width > 0 && scale < 0.72;
  const openClip = (id: string) => {
    setClipId(id);
    setPage("editor");
  };

  return (
    <Tabs.Root value={page} onValueChange={(v) => setPage(v as PageId)}>
      <Tabs.List
        aria-label="Pages de l’application"
        className="border-line bg-canvas mb-5 flex w-fit max-w-full gap-1 overflow-x-auto rounded-xl border p-1"
      >
        {TABS.map((t) => (
          <Tabs.Trigger
            key={t.id}
            value={t.id}
            className="text-muted hover:text-ink data-[state=active]:text-accent relative cursor-pointer rounded-lg px-4 py-2 text-[0.9375rem] font-medium whitespace-nowrap transition-colors"
          >
            {page === t.id && (
              <motion.span
                layoutId="tour-tab"
                className="bg-accent-surface absolute inset-0 rounded-lg"
                transition={{ type: "spring", stiffness: 500, damping: 40 }}
              />
            )}
            <span className="relative">{t.label}</span>
          </Tabs.Trigger>
        ))}
      </Tabs.List>

      <div
        ref={ref}
        className="border-line relative overflow-hidden rounded-2xl border shadow-[0_40px_90px_-40px_rgba(0,0,0,0.6)]"
        style={{ height: width === 0 ? H : compact ? undefined : H * scale }}
      >
        {width > 0 && !compact && (
          <div
            className="app-dark bg-canvas text-ink flex origin-top-left"
            style={{ width: W, height: H, transform: `scale(${scale})`, fontSize: 14 }}
          >
            <aside className="bg-nav flex w-[216px] shrink-0 flex-col p-[18px]">
              <div className="mt-3 mb-9 flex items-center gap-2.5 text-[22px] font-semibold">
                <Logo size={32} />
                ClipForge
              </div>
              <ul className="grid gap-2">
                {NAV.map(({ id, label, icon: Icon }) => (
                  <li key={label}>
                    <button
                      type="button"
                      disabled={!id}
                      onClick={() => id && setPage(id)}
                      aria-current={id === page ? "page" : undefined}
                      className={cn(
                        "text-muted enabled:hover:bg-surface flex h-[46px] w-full items-center gap-3 rounded-lg px-3 text-left text-[14px] transition-colors enabled:cursor-pointer",
                        id === page &&
                          "bg-accent-surface text-accent enabled:hover:bg-accent-surface font-semibold",
                      )}
                    >
                      <Icon className="size-[18px]" />
                      {label}
                    </button>
                  </li>
                ))}
              </ul>
              <div className="border-line mt-auto grid gap-1 border-t pt-3.5">
                <span className="text-muted text-[10px] tracking-[0.1em]">LOCAL, PAR DÉFAUT</span>
                <small className="text-muted text-[11px]">ClipForge 0.1 · GPLv3</small>
              </div>
            </aside>
            <div className="flex min-w-0 flex-1 flex-col">
              <StatusBar compact={false} />
              <Pages compact={false} setPage={setPage} clipId={clipId} openClip={openClip} />
              <div className="bg-nav text-muted h-10 shrink-0 px-[30px] py-3 text-[12px]">
                F8 : replay · F9 : enregistrement · F10 : capture d’écran
              </div>
            </div>
          </div>
        )}
        {(compact || width === 0) && (
          <div className="app-dark bg-canvas text-ink flex min-h-[28rem] flex-col">
            <StatusBar compact />
            <Pages compact setPage={setPage} clipId={clipId} openClip={openClip} />
            <div className="bg-nav text-muted shrink-0 px-4 py-3 text-[12px]">
              F8 : replay · F9 : enregistrement · F10 : capture d’écran
            </div>
          </div>
        )}
      </div>
      <p className="text-muted mt-4 text-sm">
        Rendu fidèle aux pages QML de l’application, pas une capture d’écran. Clips, miniatures et
        mesures sont des exemples ; ce que tu sauvegardes dans la démo ci-dessus apparaît ici.
      </p>
    </Tabs.Root>
  );
}
