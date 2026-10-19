import type { NextConfig } from "next";

// Export statique : aucun serveur Node requis, le dossier `out/` se publie tel quel.
// NEXT_PUBLIC_BASE_PATH sert à GitHub Pages (ex. "/ClipForge").
const config: NextConfig = {
  output: "export",
  trailingSlash: true,
  reactStrictMode: true,
  images: { unoptimized: true },
  basePath: process.env.NEXT_PUBLIC_BASE_PATH ?? "",
};

export default config;
