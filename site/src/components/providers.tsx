"use client";

import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import Lenis from "lenis";
import { MotionConfig } from "motion/react";
import { ThemeProvider } from "next-themes";
import { useEffect, useState } from "react";
import { Toaster } from "sonner";
import { useMediaQuery } from "usehooks-ts";
import { TooltipProvider } from "@/components/ui/tooltip";

function SmoothScroll() {
  const reduce = useMediaQuery("(prefers-reduced-motion: reduce)", { initializeWithValue: false });
  useEffect(() => {
    if (reduce) return;
    const lenis = new Lenis({ autoRaf: true, lerp: 0.12, anchors: { offset: -88 } });
    return () => lenis.destroy();
  }, [reduce]);
  return null;
}

export function Providers({ children }: { children: React.ReactNode }) {
  const [client] = useState(
    () => new QueryClient({ defaultOptions: { queries: { staleTime: 5 * 60_000, retry: 1 } } }),
  );
  return (
    <ThemeProvider attribute="class" defaultTheme="dark" enableSystem disableTransitionOnChange>
      <QueryClientProvider client={client}>
        <MotionConfig reducedMotion="user">
          <TooltipProvider>
            <SmoothScroll />
            {children}
            <Toaster
              position="bottom-right"
              toastOptions={{
                classNames: { description: "!text-muted" },
                style: {
                  background: "var(--surface)",
                  color: "var(--ink)",
                  border: "1px solid var(--line)",
                  fontFamily: "var(--font-sans)",
                },
              }}
            />
          </TooltipProvider>
        </MotionConfig>
      </QueryClientProvider>
    </ThemeProvider>
  );
}
