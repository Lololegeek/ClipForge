"use client";

import { useQuery } from "@tanstack/react-query";
import { fetchLatestRelease, RELEASE_SNAPSHOT } from "@/lib/github";

/** Latest published version, same query as the download panel (fetched once, shared). */
export function HeroVersion() {
  const { data } = useQuery({
    queryKey: ["latest-release"],
    queryFn: ({ signal }) => fetchLatestRelease(signal),
    initialData: RELEASE_SNAPSHOT,
    staleTime: 60_000,
  });
  const tag = (data ?? RELEASE_SNAPSHOT).tag_name.replace(/^v/, "");
  return <>Version {tag}</>;
}
