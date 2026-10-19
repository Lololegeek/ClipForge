import { cn } from "@/lib/utils";

export function Kbd({ className, ...props }: React.ComponentProps<"kbd">) {
  return (
    <kbd
      className={cn(
        "border-line bg-raised text-ink rounded-md border border-b-2 px-1.5 py-0.5 font-mono text-[0.8em] font-medium",
        className,
      )}
      {...props}
    />
  );
}
