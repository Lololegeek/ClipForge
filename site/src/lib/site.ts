export const REPO = "Lololegeek/ClipForge";
export const REPO_URL = `https://github.com/${REPO}`;
export const RELEASES_URL = `${REPO_URL}/releases/latest`;
export const ZIP_URL = `${REPO_URL}/archive/refs/heads/main.zip`;
export const DOCS_URL = (file: string) => `${REPO_URL}/blob/main/${file}`;

export const BUILD_COMMANDS = `git clone ${REPO_URL}
cd ClipForge
./scripts/bootstrap.ps1
./scripts/build.ps1
./scripts/package.ps1
# → dist/ClipForge/ClipForge.exe`;

export const SECTIONS = [
  { id: "parcours", label: "Parcours" },
  { id: "appli", label: "L’application" },
  { id: "technique", label: "Technique" },
  { id: "limites", label: "Limites" },
  { id: "faq", label: "Questions" },
  { id: "telecharger", label: "Télécharger" },
] as const;
