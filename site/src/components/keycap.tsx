"use client";

import { Tooltip } from "@/components/ui/tooltip";
import { cn } from "@/lib/utils";

export function KeyCap({
  keyName,
  label,
  hint,
  onPress,
  active,
  disabled,
}: {
  keyName: string;
  label: string;
  hint: string;
  onPress: () => void;
  active?: boolean;
  disabled?: boolean;
}) {
  return (
    <Tooltip content={hint}>
      <button
        type="button"
        onClick={onPress}
        disabled={disabled}
        aria-label={`${label} (${keyName})`}
        className={cn(
          "group border-line bg-raised text-ink hover:bg-accent-surface grid min-w-28 cursor-pointer justify-items-center gap-0.5 rounded-xl border border-b-[5px] px-4 pt-3 pb-2.5 transition-[transform,border-color,background] duration-100 active:translate-y-1 active:border-b",
          active && "border-ok/60 bg-accent-surface",
          disabled && "opacity-50",
        )}
      >
        <span
          className={cn(
            "text-accent font-mono text-2xl font-medium tracking-tight",
            active && "text-ok",
          )}
        >
          {keyName}
        </span>
        <span className="text-muted text-[0.8125rem]">{label}</span>
      </button>
    </Tooltip>
  );
}
