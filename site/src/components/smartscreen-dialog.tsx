"use client";

import * as Dialog from "@radix-ui/react-dialog";
import { X } from "lucide-react";
import { Button } from "@/components/ui/button";

const STEPS = [
  "Ouvre ClipForge-Setup-x64.exe une fois téléchargé.",
  "Si Windows affiche « Windows a protégé votre PC », clique sur Informations complémentaires.",
  "Clique sur Exécuter quand même, puis suis l’installation.",
];

/** Shown right after the download starts: explains the SmartScreen warning of an unsigned installer. */
export function SmartScreenDialog({
  open,
  onOpenChange,
}: {
  open: boolean;
  onOpenChange: (open: boolean) => void;
}) {
  return (
    <Dialog.Root open={open} onOpenChange={onOpenChange}>
      <Dialog.Portal>
        <Dialog.Overlay className="bg-canvas/70 data-[state=open]:animate-in data-[state=open]:fade-in-0 data-[state=closed]:animate-out data-[state=closed]:fade-out-0 fixed inset-0 z-50 backdrop-blur-sm" />
        <Dialog.Content className="border-line bg-nav data-[state=open]:animate-in data-[state=open]:fade-in-0 data-[state=open]:zoom-in-95 data-[state=closed]:animate-out data-[state=closed]:fade-out-0 fixed top-1/2 left-1/2 z-50 max-h-[calc(100dvh-2rem)] w-[min(32rem,calc(100vw-2rem))] -translate-x-1/2 -translate-y-1/2 overflow-y-auto rounded-xl border p-6 shadow-2xl">
          <div className="flex items-start justify-between gap-4">
            <Dialog.Title className="text-xl font-semibold">Le téléchargement a commencé</Dialog.Title>
            <Dialog.Close asChild>
              <Button variant="ghost" size="sm" aria-label="Fermer">
                <X />
              </Button>
            </Dialog.Close>
          </div>
          <Dialog.Description className="text-muted mt-2">
            Windows va sûrement te prévenir avant l’installation. C’est normal : ClipForge n’est pas
            encore signé, un certificat coûte plusieurs centaines d’euros par an.
          </Dialog.Description>

          {/* Simplified picture of the SmartScreen window, with what to click. */}
          <div
            aria-hidden="true"
            className="mt-6 rounded-lg bg-[#0063b1] p-5 text-left text-white select-none"
          >
            <p className="text-lg font-semibold">Windows a protégé votre PC</p>
            <p className="mt-2 text-xs text-white/85">
              Microsoft Defender SmartScreen a empêché le démarrage d’une application non reconnue…
            </p>
            <p className="mt-3 inline-block rounded px-1 text-xs underline ring-2 ring-[#ffd84d] ring-offset-2 ring-offset-[#0063b1]">
              ① Informations complémentaires
            </p>
            <div className="mt-5 flex justify-end gap-2">
              <span className="rounded border border-white/80 px-3 py-1 text-xs ring-2 ring-[#ffd84d] ring-offset-2 ring-offset-[#0063b1]">
                ② Exécuter quand même
              </span>
              <span className="rounded border border-white/40 px-3 py-1 text-xs text-white/70">
                Ne pas exécuter
              </span>
            </div>
          </div>

          <ol className="mt-6 grid gap-3">
            {STEPS.map((step, i) => (
              <li key={i} className="flex gap-3">
                <span className="bg-accent text-accent-ink grid size-6 shrink-0 place-items-center rounded-full text-xs font-bold">
                  {i + 1}
                </span>
                <span className="text-ink text-sm leading-6">{step}</span>
              </li>
            ))}
          </ol>

          <p className="text-muted mt-6 text-xs">
            Le code de ClipForge est public sur GitHub. L’alerte disparaîtra quand assez de monde aura
            installé ClipForge.
          </p>
          <div className="mt-6 flex justify-end">
            <Dialog.Close asChild>
              <Button variant="primary">J’ai compris</Button>
            </Dialog.Close>
          </div>
        </Dialog.Content>
      </Dialog.Portal>
    </Dialog.Root>
  );
}
