"use client";

import * as Dialog from "@radix-ui/react-dialog";
import { Menu, X } from "lucide-react";
import { motion } from "motion/react";
import Link from "next/link";
import { usePathname } from "next/navigation";
import { useState } from "react";
import { CommandMenu } from "@/components/command-menu";
import { Discord } from "@/components/icons";
import { Logo } from "@/components/logo";
import { ThemeToggle } from "@/components/theme-toggle";
import { Button } from "@/components/ui/button";
import { DISCORD_URL, SECTIONS } from "@/lib/site";

export function SiteHeader() {
  const pathname = usePathname();
  const [menu, setMenu] = useState(false);

  return (
    <header className="border-line/70 bg-canvas/85 sticky top-0 z-40 border-b backdrop-blur-md">
      <div className="mx-auto flex h-16 w-[min(72rem,100%-2rem)] items-center gap-4 sm:w-[min(72rem,100%-3rem)]">
        <Link
          href="/"
          className="flex items-center gap-2.5 text-xl font-semibold tracking-tight"
          aria-label="ClipForge, accueil"
        >
          <Logo />
          ClipForge
        </Link>

        <nav aria-label="Principal" className="ml-auto hidden items-center gap-1 md:flex">
          {SECTIONS.filter((s) => s.id !== "telecharger").map((s) => {
            const active = pathname.startsWith(s.href.slice(0, -1));
            return (
            <Link
              key={s.id}
              href={s.href}
              className="text-muted hover:text-ink aria-[current=page]:text-ink relative rounded-md px-3 py-2 text-[0.9375rem] transition-colors"
              aria-current={active ? "page" : undefined}
            >
              {s.label}
              {active && (
                <motion.span
                  layoutId="nav-underline"
                  className="bg-accent absolute inset-x-3 -bottom-px h-0.5 rounded-full"
                  transition={{ type: "spring", stiffness: 500, damping: 40 }}
                />
              )}
            </Link>
            );
          })}
          <Link
            href="/fonctionnalites/"
            className="text-muted hover:text-ink aria-[current=page]:text-ink rounded-md px-3 py-2 text-[0.9375rem] transition-colors"
            aria-current={pathname.startsWith("/fonctionnalites") ? "page" : undefined}
          >
            Fonctionnalités
          </Link>
          <Link
            href="/changelog/"
            className="text-muted hover:text-ink aria-[current=page]:text-ink rounded-md px-3 py-2 text-[0.9375rem] transition-colors"
            aria-current={pathname.startsWith("/changelog") ? "page" : undefined}
          >
            Changelog
          </Link>
        </nav>

        <div className="ml-auto flex items-center gap-2 md:ml-2">
          <CommandMenu />
          <Button asChild variant="ghost" size="sm" className="size-9 px-0">
            <a href={DISCORD_URL} target="_blank" rel="noopener noreferrer" aria-label="Discord ClipForge">
              <Discord />
            </a>
          </Button>
          <ThemeToggle />
          <Button asChild variant="primary" size="sm" className="hidden sm:inline-flex">
            <Link href="/telechargement/">Télécharger</Link>
          </Button>

          <Dialog.Root open={menu} onOpenChange={setMenu}>
            <Dialog.Trigger asChild>
              <Button
                variant="ghost"
                size="sm"
                className="size-9 px-0 md:hidden"
                aria-label="Ouvrir le menu"
              >
                <Menu />
              </Button>
            </Dialog.Trigger>
            <Dialog.Portal>
              <Dialog.Overlay className="bg-canvas/70 data-[state=open]:animate-in data-[state=open]:fade-in-0 data-[state=closed]:animate-out data-[state=closed]:fade-out-0 fixed inset-0 z-50 backdrop-blur-sm" />
              <Dialog.Content className="border-line bg-nav data-[state=open]:animate-in data-[state=open]:slide-in-from-right data-[state=closed]:animate-out data-[state=closed]:slide-out-to-right fixed inset-y-0 right-0 z-50 flex w-[min(20rem,85vw)] flex-col gap-2 border-l p-6">
                <div className="mb-6 flex items-center justify-between">
                  <Dialog.Title className="text-lg font-semibold">Menu</Dialog.Title>
                  <Dialog.Close asChild>
                    <Button
                      variant="ghost"
                      size="sm"
                      className="size-9 px-0"
                      aria-label="Fermer le menu"
                    >
                      <X />
                    </Button>
                  </Dialog.Close>
                </div>
                <Dialog.Description className="sr-only">Navigation du site</Dialog.Description>
                {SECTIONS.map((s) => (
                  <Link
                    key={s.id}
                    href={s.href}
                    onClick={() => setMenu(false)}
                    className="text-ink hover:bg-raised rounded-lg px-3 py-3 text-lg"
                  >
                    {s.label}
                  </Link>
                ))}
                <Link
                  href="/fonctionnalites/"
                  onClick={() => setMenu(false)}
                  className="text-ink hover:bg-raised rounded-lg px-3 py-3 text-lg"
                >
                  Fonctionnalités détaillées
                </Link>
                <Link
                  href="/changelog/"
                  onClick={() => setMenu(false)}
                  className="text-ink hover:bg-raised rounded-lg px-3 py-3 text-lg"
                >
                  Changelog
                </Link>
                <Link
                  href="/licences/"
                  onClick={() => setMenu(false)}
                  className="text-ink hover:bg-raised rounded-lg px-3 py-3 text-lg"
                >
                  Licences
                </Link>
                <a
                  href={DISCORD_URL}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="text-ink hover:bg-raised rounded-lg px-3 py-3 text-lg"
                >
                  Discord
                </a>
              </Dialog.Content>
            </Dialog.Portal>
          </Dialog.Root>
        </div>
      </div>
    </header>
  );
}
