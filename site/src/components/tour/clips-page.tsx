"use client";

import * as ToggleGroup from "@radix-ui/react-toggle-group";
import { useAutoAnimate } from "@formkit/auto-animate/react";
import { Star } from "lucide-react";
import { useMemo, useState } from "react";
import { Thumb } from "@/components/tour/thumb";
import { AppButton, PageTitle } from "@/components/tour/ui";
import { clock } from "@/lib/format";
import { useDemo } from "@/lib/store";
import { cn } from "@/lib/utils";

type Sort = "date" | "duree" | "nom";

const toggleItem =
  "h-8 cursor-pointer rounded-lg px-3 text-[13px] text-muted transition-colors hover:text-ink data-[state=on]:bg-accent-surface data-[state=on]:text-accent";

export function ClipsPage({ onOpen }: { onOpen: (id: string) => void }) {
  const clips = useDemo((s) => s.clips);
  const toggleFavorite = useDemo((s) => s.toggleFavorite);
  const [query, setQuery] = useState("");
  const [sort, setSort] = useState<Sort>("date");
  const [favOnly, setFavOnly] = useState(false);
  const [view, setView] = useState<"grille" | "liste">("grille");
  const [listRef] = useAutoAnimate<HTMLDivElement>({ duration: 200 });

  const shown = useMemo(() => {
    const q = query.trim().toLowerCase();
    const filtered = clips.filter(
      (c) => (!favOnly || c.favorite) && (!q || `${c.name} ${c.game}`.toLowerCase().includes(q)),
    );
    if (sort === "duree") return [...filtered].sort((a, b) => b.seconds - a.seconds);
    if (sort === "nom") return [...filtered].sort((a, b) => a.name.localeCompare(b.name, "fr"));
    return filtered;
  }, [clips, query, sort, favOnly]);

  return (
    <div className="@container grid content-start gap-4">
      <div className="flex flex-wrap items-center gap-3">
        <PageTitle count={clips.length}>Mes clips</PageTitle>
        <div className="ml-auto flex gap-2">
          <AppButton disabled>Importer</AppButton>
          <AppButton disabled>Actualiser</AppButton>
        </div>
      </div>

      <div className="flex flex-wrap items-center gap-2">
        <input
          id="clip-search"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          placeholder="Rechercher un clip…"
          aria-label="Rechercher un clip"
          className="bg-surface text-ink placeholder:text-muted focus-visible:outline-accent h-9 w-full min-w-0 rounded-lg px-3 text-[13px] outline-none focus-visible:outline-2 @md:w-56"
        />
        <ToggleGroup.Root
          type="single"
          value={sort}
          onValueChange={(v) => v && setSort(v as Sort)}
          aria-label="Trier par"
          className="bg-surface flex rounded-lg p-0.5"
        >
          <ToggleGroup.Item value="date" className={toggleItem}>
            Date
          </ToggleGroup.Item>
          <ToggleGroup.Item value="duree" className={toggleItem}>
            Durée
          </ToggleGroup.Item>
          <ToggleGroup.Item value="nom" className={toggleItem}>
            Nom
          </ToggleGroup.Item>
        </ToggleGroup.Root>
        <button
          type="button"
          aria-pressed={favOnly}
          onClick={() => setFavOnly((v) => !v)}
          className={cn(
            "bg-surface text-muted hover:text-ink h-9 cursor-pointer rounded-lg px-3 text-[13px] transition-colors",
            favOnly && "bg-accent-surface text-accent",
          )}
        >
          ★ Favoris
        </button>
        <ToggleGroup.Root
          type="single"
          value={view}
          onValueChange={(v) => v && setView(v as "grille" | "liste")}
          aria-label="Affichage"
          className="bg-surface ml-auto flex rounded-lg p-0.5"
        >
          <ToggleGroup.Item value="grille" className={toggleItem}>
            Grille
          </ToggleGroup.Item>
          <ToggleGroup.Item value="liste" className={toggleItem}>
            Liste
          </ToggleGroup.Item>
        </ToggleGroup.Root>
      </div>

      {shown.length === 0 && (
        <p className="text-muted py-10 text-center text-[14px]">
          Aucun clip trouvé. Sauvegarde un replay ou importe une vidéo.
        </p>
      )}

      <div
        ref={listRef}
        className={cn(
          "grid gap-4",
          view === "grille" ? "grid-cols-1 @md:grid-cols-2 @3xl:grid-cols-3" : "grid-cols-1 gap-1",
        )}
      >
        {shown.slice(0, view === "grille" ? 6 : 8).map((c) =>
          view === "grille" ? (
            <div key={c.id} className="group relative">
              <button
                type="button"
                onClick={() => onOpen(c.id)}
                className="block w-full cursor-pointer text-left"
                aria-label={`Ouvrir ${c.name} dans l’éditeur`}
              >
                <Thumb tone={c.tone} className="aspect-video rounded-[10px]">
                  <span className="bg-canvas/80 text-ink absolute right-2 bottom-2 rounded-[5px] px-1.5 py-0.5 font-mono text-[12px]">
                    {clock(c.seconds)}
                  </span>
                </Thumb>
                <b className="mt-2 block truncate text-[14px] font-medium">
                  {c.favorite ? "★  " : ""}
                  {c.name}
                </b>
                <small className="text-muted text-[13px]">{c.game}</small>
              </button>
              <button
                type="button"
                onClick={() => toggleFavorite(c.id)}
                aria-pressed={c.favorite}
                aria-label={c.favorite ? "Retirer des favoris" : "Ajouter aux favoris"}
                className="bg-canvas/75 text-ink absolute top-2 right-2 grid size-8 cursor-pointer place-items-center rounded-lg opacity-0 transition-opacity group-hover:opacity-100 focus-visible:opacity-100 pointer-coarse:opacity-100"
              >
                <Star className={cn("size-4", c.favorite && "text-accent fill-current")} />
              </button>
            </div>
          ) : (
            <div
              key={c.id}
              className="hover:bg-surface flex items-center gap-3 rounded-[7px] px-3 py-2"
            >
              <Thumb tone={c.tone} className="aspect-video w-20 shrink-0 rounded-md" />
              <button
                type="button"
                onClick={() => onOpen(c.id)}
                className="min-w-0 flex-1 cursor-pointer text-left"
              >
                <b className="block truncate text-[14px] font-medium">
                  {c.favorite ? "★  " : ""}
                  {c.name}
                </b>
                <small className="text-muted text-[13px]">
                  {c.game} · {c.seconds} s
                </small>
              </button>
              <button
                type="button"
                onClick={() => toggleFavorite(c.id)}
                aria-pressed={c.favorite}
                aria-label={c.favorite ? "Retirer des favoris" : "Ajouter aux favoris"}
                className="text-muted hover:text-ink grid size-8 cursor-pointer place-items-center rounded-lg"
              >
                <Star className={cn("size-4", c.favorite && "text-accent fill-current")} />
              </button>
            </div>
          ),
        )}
      </div>
    </div>
  );
}
