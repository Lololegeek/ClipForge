import type { Metadata } from "next";
import { SponsorRedirect } from "@/components/sponsor-redirect";
import { SPONSORS } from "@/lib/sponsors";

export const dynamicParams = false;

export function generateStaticParams() {
  return SPONSORS.map(({ slug }) => ({ slug }));
}

export const metadata: Metadata = {
  title: "Redirection",
  robots: { index: false, follow: false },
};

export default async function SponsorPage({ params }: { params: Promise<{ slug: string }> }) {
  const { slug } = await params;
  return <SponsorRedirect slug={slug} />;
}
