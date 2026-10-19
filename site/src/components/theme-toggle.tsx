"use client";

import { Moon, Sun } from "lucide-react";
import { useTheme } from "next-themes";
import { useIsClient } from "usehooks-ts";
import { Button } from "@/components/ui/button";
import { Tooltip } from "@/components/ui/tooltip";

export function ThemeToggle() {
  const { resolvedTheme, setTheme } = useTheme();
  const isClient = useIsClient();
  const dark = !isClient || resolvedTheme === "dark";
  return (
    <Tooltip content={dark ? "Passer au thème clair" : "Passer au thème sombre"} side="bottom">
      <Button
        variant="ghost"
        size="sm"
        className="size-9 px-0"
        aria-label={dark ? "Passer au thème clair" : "Passer au thème sombre"}
        onClick={() => setTheme(dark ? "light" : "dark")}
      >
        {dark ? <Sun /> : <Moon />}
      </Button>
    </Tooltip>
  );
}
