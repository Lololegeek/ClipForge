import { cn } from "@/lib/utils";

/** Boutons de la fenêtre simulée : mêmes rôles que ActionButton.qml. */
export function AppButton({
  primary,
  className,
  ...props
}: React.ComponentProps<"button"> & { primary?: boolean }) {
  return (
    <button
      type="button"
      className={cn(
        "inline-flex h-10 cursor-pointer items-center justify-center rounded-lg px-4 text-[14px] font-medium whitespace-nowrap transition-colors disabled:cursor-default disabled:opacity-45",
        primary
          ? "bg-accent text-accent-ink font-semibold hover:brightness-110"
          : "bg-raised text-ink hover:bg-line",
        className,
      )}
      {...props}
    />
  );
}

export function PageTitle({
  children,
  count,
}: {
  children: React.ReactNode;
  count?: React.ReactNode;
}) {
  return (
    <div className="flex items-baseline gap-3">
      <h3 className="text-[28px] leading-tight font-semibold tracking-tight">{children}</h3>
      {count !== undefined && <span className="text-muted font-mono text-[14px]">{count}</span>}
    </div>
  );
}
