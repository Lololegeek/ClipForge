import Link from "next/link";
import { Button } from "@/components/ui/button";

export default function NotFound() {
  return (
    <div className="mx-auto grid w-[min(40rem,100%-2rem)] justify-items-start gap-5 py-32">
      <p className="text-accent font-mono text-sm tracking-widest uppercase">Erreur 404</p>
      <h1 className="text-5xl font-bold tracking-tight">Ce moment n’a pas été sauvegardé.</h1>
      <p className="text-muted text-lg">La page demandée n’existe pas ou a été déplacée.</p>
      <Button asChild variant="primary">
        <Link href="/">Retour à l’accueil</Link>
      </Button>
    </div>
  );
}
