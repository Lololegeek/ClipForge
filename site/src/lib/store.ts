import { format } from "date-fns";
import { create } from "zustand";

export type Clip = {
  id: string;
  name: string;
  seconds: number;
  game: string;
  favorite: boolean;
  /** `demo` : sauvegardé depuis la démo du site, `example` : donnée d'exemple. */
  origin: "demo" | "example";
  tone: number;
};

const EXAMPLES: Clip[] = [
  {
    id: "ex-1",
    name: "Clutch 1v3",
    seconds: 30,
    game: "Arena",
    favorite: true,
    origin: "example",
    tone: 0,
  },
  {
    id: "ex-2",
    name: "Replay 2026-10-18 21-17-44",
    seconds: 45,
    game: "Route 66",
    favorite: false,
    origin: "example",
    tone: 1,
  },
  {
    id: "ex-3",
    name: "Fin de partie",
    seconds: 60,
    game: "Arena",
    favorite: false,
    origin: "example",
    tone: 2,
  },
  {
    id: "ex-4",
    name: "Replay 2026-10-17 20-52-09",
    seconds: 18,
    game: "Vidéo locale",
    favorite: false,
    origin: "example",
    tone: 3,
  },
  {
    id: "ex-5",
    name: "Course finale",
    seconds: 72,
    game: "Route 66",
    favorite: true,
    origin: "example",
    tone: 4,
  },
  {
    id: "ex-6",
    name: "Replay 2026-10-16 19-08-31",
    seconds: 25,
    game: "Vidéo locale",
    favorite: false,
    origin: "example",
    tone: 5,
  },
];

/** La démo remplit le tampon plus vite que le temps réel. */
export const DEMO_SPEED = 5;

type State = {
  replaySeconds: number;
  buffer: number;
  active: boolean;
  recording: boolean;
  recSeconds: number;
  flash: number;
  lastSave: { at: number; seconds: number } | null;
  clips: Clip[];
  setReplaySeconds: (s: number) => void;
  toggleActive: () => void;
  toggleRecording: () => void;
  tick: (dt: number) => void;
  saveReplay: () => Clip | null;
  screenshot: () => void;
  toggleFavorite: (id: string) => void;
};

export const useDemo = create<State>((set, get) => ({
  replaySeconds: 60,
  buffer: 24,
  active: true,
  recording: false,
  recSeconds: 0,
  flash: 0,
  lastSave: null,
  clips: EXAMPLES,
  setReplaySeconds: (s) => set((st) => ({ replaySeconds: s, buffer: Math.min(st.buffer, s) })),
  toggleActive: () => set((st) => ({ active: !st.active, buffer: st.active ? 0 : st.buffer })),
  toggleRecording: () => set((st) => ({ recording: !st.recording, recSeconds: 0 })),
  tick: (dt) =>
    set((st) => ({
      buffer: st.active ? Math.min(st.replaySeconds, st.buffer + dt * DEMO_SPEED) : st.buffer,
      recSeconds: st.recording ? st.recSeconds + dt : st.recSeconds,
    })),
  saveReplay: () => {
    const { active, buffer, clips } = get();
    if (!active || buffer < 1) return null;
    const seconds = Math.floor(buffer);
    const clip: Clip = {
      id: `demo-${Date.now()}`,
      name: `Replay ${format(new Date(), "yyyy-MM-dd HH-mm-ss")}`,
      seconds,
      game: "Démo du site",
      favorite: false,
      origin: "demo",
      tone: clips.length % 6,
    };
    set({ clips: [clip, ...clips], lastSave: { at: Date.now(), seconds } });
    return clip;
  },
  screenshot: () => set((st) => ({ flash: st.flash + 1 })),
  toggleFavorite: (id) =>
    set((st) => ({
      clips: st.clips.map((c) => (c.id === id ? { ...c, favorite: !c.favorite } : c)),
    })),
}));
