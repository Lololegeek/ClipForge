import type { MetadataRoute } from "next";
import { SITE_URL } from "@/lib/site-url";

export const dynamic = "force-static";

const PAGES = [
  "",
  "telechargement",
  "fonctionnalites",
  "application",
  "parcours",
  "faq",
  "changelog",
  "technique",
  "limites",
  "licences",
];

export default function sitemap(): MetadataRoute.Sitemap {
  return PAGES.map((page) => ({
    url: `${SITE_URL}/${page ? `${page}/` : ""}`,
    changeFrequency: "weekly",
    priority: page ? 0.7 : 1,
  }));
}
