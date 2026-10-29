export const REPO = "Lololegeek/ClipForge";
export const REPO_URL = `https://github.com/${REPO}`;
export const RELEASES_URL = `${REPO_URL}/releases/latest`;
export const ZIP_URL = `${REPO_URL}/archive/refs/heads/main.zip`;
export const DOCS_URL = (file: string) => `${REPO_URL}/blob/main/${file}`;
export const DISCORD_URL = "https://discord.gg/asj8TNXG4F";
export const YOUTUBE_URL = "https://www.youtube.com/channel/UCu_NfhXChxeLlQRaB9oJK_w";

export const BUILD_COMMANDS = `git clone ${REPO_URL}
cd ClipForge
./scripts/bootstrap.ps1
./scripts/build.ps1
./scripts/package.ps1
# → dist/ClipForge/ClipForge.exe`;

export const SECTIONS = [
  { id: "parcours", label: "Parcours", href: "/parcours/" },
  { id: "appli", label: "L’application", href: "/application/" },
  { id: "technique", label: "Technique", href: "/technique/" },
  { id: "limites", label: "Limites", href: "/limites/" },
  { id: "faq", label: "Questions", href: "/faq/" },
  { id: "telecharger", label: "Télécharger", href: "/telechargement/" },
] as const;
