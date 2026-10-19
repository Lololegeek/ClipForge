import { cn } from "@/lib/utils";

const ART = [
  {
    bg: "linear-gradient(180deg,#2b2f5c,#4b3f7a 60%,#1d1c30)",
    shape: "polygon(0 100%,0 62%,18% 30%,34% 58%,52% 18%,70% 52%,86% 34%,100% 60%,100% 100%)",
  },
  { bg: "linear-gradient(120deg,#123b3f,#1f6a62 50%,#0f1f26)", lines: true },
  {
    bg: "radial-gradient(circle at 70% 30%,#e4a2ff55 0,transparent 40%),linear-gradient(160deg,#3a1f4a,#171822)",
  },
  { bg: "linear-gradient(200deg,#54421f,#2a2118 60%,#14151f)", sun: true },
  { bg: "linear-gradient(135deg,#1b2e57,#22345f 45%,#101119)" },
  { bg: "repeating-linear-gradient(90deg,#232638 0 28px,#1b1d2b 28px 56px)" },
];

/** Miniature abstraite : le site n'utilise aucune capture de jeu réelle. */
export function Thumb({
  tone,
  className,
  children,
}: {
  tone: number;
  className?: string;
  children?: React.ReactNode;
}) {
  const art = ART[tone % ART.length];
  return (
    <div
      className={cn("relative overflow-hidden", className)}
      style={{ background: art.bg }}
      aria-hidden
    >
      {art.shape && (
        <div
          className="absolute inset-x-0 bottom-0 h-2/5 bg-[#12131c]"
          style={{ clipPath: art.shape }}
        />
      )}
      {art.lines && (
        <div className="absolute -inset-x-[10%] bottom-[18%] h-[3px] -rotate-6 bg-white/35 shadow-[0_18px_0_rgba(255,255,255,0.18),0_36px_0_rgba(255,255,255,0.1)]" />
      )}
      {art.sun && (
        <div className="absolute top-1/4 left-1/5 size-8 rounded-full bg-[#ffd89a]/75 blur-[1px]" />
      )}
      {children}
    </div>
  );
}
