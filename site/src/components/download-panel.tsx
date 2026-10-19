"use client";

import { useQuery } from "@tanstack/react-query";
import { formatDistanceToNow } from "date-fns";
import { fr } from "date-fns/locale";
import { Download, RefreshCw } from "lucide-react";
import Link from "next/link";
import { CodeBlock } from "@/components/code-block";
import { Github, Windows } from "@/components/icons";
import { Section, SectionTitle } from "@/components/sections";
import { Button } from "@/components/ui/button";
import { formatBytes } from "@/lib/format";
import { fetchLatestRelease, pickInstaller } from "@/lib/github";
import { BUILD_COMMANDS, DOCS_URL, RELEASES_URL, REPO_URL, ZIP_URL } from "@/lib/site";

export function DownloadPanel() {
  const { data, isPending, isError, refetch, isFetching } = useQuery({
    queryKey: ["latest-release"],
    queryFn: ({ signal }) => fetchLatestRelease(signal),
  });
  const installer = data ? pickInstaller(data) : null;

  return (
    <div className="grid gap-10 lg:grid-cols-2 lg:gap-16">
      <div className="min-w-0">
        {isPending && (
          <div
            className="grid gap-4"
            aria-busy="true"
            aria-label="Recherche de la dernière version"
          >
            <div className="bg-raised h-14 w-72 animate-pulse rounded-lg" />
            <div className="bg-raised h-5 w-56 animate-pulse rounded" />
          </div>
        )}

        {data && (
          <div>
            <div className="flex flex-wrap gap-3">
              <Button asChild variant="primary" size="lg">
                <a
                  href={installer?.browser_download_url ?? data.html_url}
                  {...(installer ? {} : { target: "_blank", rel: "noopener noreferrer" })}
                >
                  {installer ? <Download /> : <Windows />}{" "}
                  {installer ? `Télécharger ${data.tag_name}` : `Voir la version ${data.tag_name}`}
                </a>
              </Button>
              <Button asChild variant="secondary" size="lg">
                <a href={RELEASES_URL} target="_blank" rel="noopener noreferrer">
                  Toutes les versions
                </a>
              </Button>
            </div>
            <p className="text-muted mt-4 text-sm">
              {installer && (
                <>
                  {installer.name} · {formatBytes(installer.size)} ·{" "}
                </>
              )}
              publiée{" "}
              {data.published_at
                ? formatDistanceToNow(new Date(data.published_at), { addSuffix: true, locale: fr })
                : "récemment"}
            </p>
          </div>
        )}

        {data === null && (
          <div>
            <p className="text-ink max-w-md text-lg">
              Aucune version téléchargeable pour l’instant.
            </p>
            <p className="text-muted mt-3 max-w-md">
              ClipForge embarque OBS, FFmpeg et Qt. Avant de publier un installateur, il faut réunir
              les sources correspondantes de chaque composant, comme l’exige la GPL. En attendant,
              tu peux compiler ClipForge depuis les sources.
            </p>
            <div className="mt-6 flex flex-wrap gap-3">
              <Button asChild variant="primary" size="lg">
                <a href={ZIP_URL}>
                  <Download /> Code source (.zip)
                </a>
              </Button>
              <Button asChild variant="secondary" size="lg">
                <a href={REPO_URL} target="_blank" rel="noopener noreferrer">
                  <Github /> Dépôt GitHub
                </a>
              </Button>
            </div>
            <p className="text-muted mt-4 text-sm">
              Suis la page des{" "}
              <a
                className="text-accent underline underline-offset-4"
                href={RELEASES_URL}
                target="_blank"
                rel="noopener noreferrer"
              >
                versions
              </a>{" "}
              pour être prévenu de la première publication.
            </p>
          </div>
        )}

        {isError && (
          <div>
            <p className="text-ink text-lg">Impossible de vérifier la dernière version.</p>
            <p className="text-muted mt-2">
              GitHub n’a pas répondu. Tu peux réessayer, ou ouvrir directement la page des versions.
            </p>
            <div className="mt-5 flex flex-wrap gap-3">
              <Button variant="secondary" onClick={() => refetch()} disabled={isFetching}>
                <RefreshCw className={isFetching ? "animate-spin" : ""} /> Réessayer
              </Button>
              <Button asChild variant="primary">
                <a href={RELEASES_URL} target="_blank" rel="noopener noreferrer">
                  Page des versions
                </a>
              </Button>
            </div>
          </div>
        )}

        <ul className="text-muted mt-10 grid max-w-md gap-2 text-sm">
          <li>Windows x64, installation par utilisateur, sans droits administrateur.</li>
          <li>Aucun compte, aucune activation en ligne.</li>
          <li>Windows 10 et machine vierge : pas encore vérifiés.</li>
        </ul>
      </div>

      <div className="min-w-0">
        <h3 className="mb-3 text-xl font-semibold">Compiler depuis les sources</h3>
        <CodeBlock code={BUILD_COMMANDS} />
        <p className="text-muted mt-4 text-sm">
          Prérequis : Visual Studio (MSVC x64) et Windows SDK, CMake, Ninja, Python (téléchargement
          des dépendances seulement), 7-Zip, FFmpeg/FFprobe. Détails dans{" "}
          <a
            className="text-accent underline underline-offset-4"
            href={DOCS_URL("docs/BUILDING.md")}
            target="_blank"
            rel="noopener noreferrer"
          >
            docs/BUILDING.md
          </a>{" "}
          et la page{" "}
          <Link className="text-accent underline underline-offset-4" href="/telechargement/">
            téléchargement
          </Link>
          .
        </p>
      </div>
    </div>
  );
}

export function DownloadSection() {
  return (
    <Section id="telecharger" className="border-line bg-nav border-t">
      <SectionTitle>Télécharger.</SectionTitle>
      <div className="mt-12">
        <DownloadPanel />
      </div>
    </Section>
  );
}
