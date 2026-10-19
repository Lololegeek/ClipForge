"use client";

import { Command } from "cmdk";
import { Download, FileText, Moon, Play, Scale, Search, Terminal } from "lucide-react";
import { useRouter } from "next/navigation";
import { useTheme } from "next-themes";
import { useState } from "react";
import { useHotkeys } from "react-hotkeys-hook";
import { toast } from "sonner";
import { useCopyToClipboard } from "usehooks-ts";
import { Github } from "@/components/icons";
import { Button } from "@/components/ui/button";
import { Kbd } from "@/components/ui/kbd";
import { saveReplayWithToast } from "@/lib/actions";
import { BUILD_COMMANDS, REPO_URL, SECTIONS } from "@/lib/site";

const itemClass =
  "flex cursor-pointer items-center gap-3 rounded-lg px-3 py-2.5 text-[0.9375rem] text-muted data-[selected=true]:bg-accent-surface data-[selected=true]:text-ink [&_svg]:size-4 [&_svg]:shrink-0";

export function CommandMenu() {
  const [open, setOpen] = useState(false);
  const router = useRouter();
  const { resolvedTheme, setTheme } = useTheme();
  const [, copy] = useCopyToClipboard();

  useHotkeys(
    "mod+k",
    (e) => {
      e.preventDefault();
      setOpen((o) => !o);
    },
    { enableOnFormTags: true },
  );

  const run = (fn: () => void) => () => {
    setOpen(false);
    fn();
  };

  return (
    <>
      <Button
        variant="secondary"
        size="sm"
        className="text-muted gap-2 px-3 font-medium"
        onClick={() => setOpen(true)}
        aria-label="Ouvrir la palette de commandes"
      >
        <Search />
        <span className="hidden lg:inline">Rechercher</span>
        <Kbd className="hidden lg:inline">Ctrl K</Kbd>
      </Button>
      <Command.Dialog
        open={open}
        onOpenChange={setOpen}
        label="Palette de commandes"
        overlayClassName="fixed inset-0 z-50 bg-canvas/70 backdrop-blur-sm data-[state=open]:animate-in data-[state=open]:fade-in-0"
        contentClassName="fixed left-1/2 top-[18vh] z-50 w-[min(34rem,calc(100%-2rem))] -translate-x-1/2 overflow-hidden rounded-2xl border border-line bg-surface shadow-2xl data-[state=open]:animate-in data-[state=open]:fade-in-0 data-[state=open]:zoom-in-95"
      >
        <Command.Input
          placeholder="Aller à, télécharger, essayer F8…"
          className="border-line text-ink placeholder:text-muted w-full border-b bg-transparent px-5 py-4 text-base outline-none"
        />
        <Command.List className="max-h-80 overflow-y-auto p-2">
          <Command.Empty className="text-muted px-3 py-8 text-center text-sm">
            Aucun résultat.
          </Command.Empty>
          <Command.Group
            heading="Sur cette page"
            className="[&_[cmdk-group-heading]]:text-muted [&_[cmdk-group-heading]]:px-3 [&_[cmdk-group-heading]]:py-2 [&_[cmdk-group-heading]]:text-xs [&_[cmdk-group-heading]]:tracking-widest [&_[cmdk-group-heading]]:uppercase"
          >
            {SECTIONS.map((s) => (
              <Command.Item
                key={s.id}
                value={`section ${s.label}`}
                className={itemClass}
                onSelect={run(() => router.push(`/#${s.id}`))}
              >
                <FileText /> {s.label}
              </Command.Item>
            ))}
          </Command.Group>
          <Command.Group
            heading="Pages"
            className="[&_[cmdk-group-heading]]:text-muted [&_[cmdk-group-heading]]:px-3 [&_[cmdk-group-heading]]:py-2 [&_[cmdk-group-heading]]:text-xs [&_[cmdk-group-heading]]:tracking-widest [&_[cmdk-group-heading]]:uppercase"
          >
            <Command.Item
              value="page fonctionnalités périmètre version 0.1"
              className={itemClass}
              onSelect={run(() => router.push("/fonctionnalites/"))}
            >
              <FileText /> Fonctionnalités et limites détaillées
            </Command.Item>
            <Command.Item
              value="page télécharger installation compiler"
              className={itemClass}
              onSelect={run(() => router.push("/telechargement/"))}
            >
              <Download /> Télécharger et compiler
            </Command.Item>
            <Command.Item
              value="page licences tierces GPL"
              className={itemClass}
              onSelect={run(() => router.push("/licences/"))}
            >
              <Scale /> Licences tierces
            </Command.Item>
          </Command.Group>
          <Command.Group
            heading="Actions"
            className="[&_[cmdk-group-heading]]:text-muted [&_[cmdk-group-heading]]:px-3 [&_[cmdk-group-heading]]:py-2 [&_[cmdk-group-heading]]:text-xs [&_[cmdk-group-heading]]:tracking-widest [&_[cmdk-group-heading]]:uppercase"
          >
            <Command.Item
              value="action essayer F8 sauvegarder replay démo"
              className={itemClass}
              onSelect={run(() => {
                router.push("/");
                saveReplayWithToast();
              })}
            >
              <Play /> Essayer F8 dans la démo
            </Command.Item>
            <Command.Item
              value="action copier commandes build compilation"
              className={itemClass}
              onSelect={run(() => {
                void copy(BUILD_COMMANDS).then((ok) => ok && toast.success("Commandes copiées"));
              })}
            >
              <Terminal /> Copier les commandes de compilation
            </Command.Item>
            <Command.Item
              value="action thème sombre clair"
              className={itemClass}
              onSelect={run(() => setTheme(resolvedTheme === "dark" ? "light" : "dark"))}
            >
              <Moon /> Changer de thème
            </Command.Item>
            <Command.Item
              value="action github code source dépôt"
              className={itemClass}
              onSelect={run(() => window.open(REPO_URL, "_blank", "noopener"))}
            >
              <Github /> Ouvrir le dépôt GitHub
            </Command.Item>
          </Command.Group>
        </Command.List>
      </Command.Dialog>
    </>
  );
}
