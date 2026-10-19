import { readFile } from "node:fs/promises";
import path from "node:path";

/** Lit un fichier Markdown du dépôt au moment du build. Une seule source de vérité. */
export async function readRepoDoc(relative: string) {
  return readFile(path.join(process.cwd(), "..", relative), "utf8");
}
