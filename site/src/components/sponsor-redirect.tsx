"use client";

import Link from "next/link";
import { useEffect } from "react";
import { setSponsor, track } from "@/lib/analytics";

const BASE_PATH = process.env.NEXT_PUBLIC_BASE_PATH ?? "";

export function SponsorRedirect({ slug }: { slug: string }) {
  const target = `/?utm_source=${slug}&utm_medium=sponso&utm_campaign=sponso`;

  useEffect(() => {
    setSponsor(slug);
    track("sponso-clic", { sponsor: slug }).then(() => window.location.replace(BASE_PATH + target));
  }, [slug, target]);

  return (
    <div className="mx-auto grid w-[min(40rem,100%-2rem)] justify-items-start gap-5 py-32">
      <p className="text-muted text-lg">Redirection vers ClipForge…</p>
      <Link className="text-accent underline underline-offset-4" href={target}>
        Continuer
      </Link>
    </div>
  );
}
