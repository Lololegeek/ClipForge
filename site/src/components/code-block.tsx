"use client";

import * as ScrollArea from "@radix-ui/react-scroll-area";
import { Check, Copy } from "lucide-react";
import { Highlight, type PrismTheme } from "prism-react-renderer";
import { useState } from "react";
import { toast } from "sonner";
import { useCopyToClipboard } from "usehooks-ts";
import { Button } from "@/components/ui/button";

// Couleurs via variables CSS : lisible dans les deux thèmes.
const theme: PrismTheme = {
  plain: { color: "var(--ink)", backgroundColor: "transparent" },
  styles: [
    { types: ["comment"], style: { color: "var(--muted)", fontStyle: "italic" } },
    { types: ["function", "builtin", "keyword"], style: { color: "var(--accent)" } },
    { types: ["string", "attr-value"], style: { color: "var(--ok)" } },
    { types: ["operator", "punctuation"], style: { color: "var(--muted)" } },
  ],
};

export function CodeBlock({ code, label = "PowerShell 7" }: { code: string; label?: string }) {
  const [, copy] = useCopyToClipboard();
  const [done, setDone] = useState(false);

  async function onCopy() {
    const ok = await copy(code);
    if (!ok) {
      toast.error("Copie impossible", {
        description: "Sélectionne le texte et copie-le à la main.",
      });
      return;
    }
    setDone(true);
    toast.success("Commandes copiées");
    setTimeout(() => setDone(false), 1800);
  }

  return (
    <div className="border-line bg-surface overflow-hidden rounded-xl border">
      <div className="border-line flex items-center justify-between border-b py-2 pr-2.5 pl-4">
        <span className="text-muted font-mono text-[0.8125rem]">{label}</span>
        <Button
          variant="ghost"
          size="sm"
          onClick={onCopy}
          className="h-8 gap-1.5 px-2.5 text-[0.8125rem]"
        >
          {done ? <Check /> : <Copy />} {done ? "Copié" : "Copier"}
        </Button>
      </div>
      <ScrollArea.Root>
        <ScrollArea.Viewport>
          <Highlight code={code} language="bash" theme={theme}>
            {({ tokens, getLineProps, getTokenProps }) => (
              <pre className="p-4 font-mono text-sm leading-7">
                {tokens.map((line, i) => (
                  <div key={i} {...getLineProps({ line })}>
                    {line.map((token, k) => (
                      <span key={k} {...getTokenProps({ token })} />
                    ))}
                  </div>
                ))}
              </pre>
            )}
          </Highlight>
        </ScrollArea.Viewport>
        <ScrollArea.Scrollbar
          orientation="horizontal"
          className="flex h-2 touch-none p-0.5 select-none"
        >
          <ScrollArea.Thumb className="bg-line relative flex-1 rounded-full" />
        </ScrollArea.Scrollbar>
      </ScrollArea.Root>
    </div>
  );
}
