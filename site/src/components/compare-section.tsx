import { Check, Minus, X } from "lucide-react";
import { Section, SectionTitle } from "@/components/sections";

type Mark = "yes" | "no" | "partial";

const TOOLS = ["ClipForge", "NVIDIA App", "AMD Adrenalin", "Xbox Game Bar", "Medal", "Outplayed"];

const ROWS: { label: string; marks: Mark[]; note?: string }[] = [
  {
    label: "Toutes les cartes graphiques",
    marks: ["yes", "no", "no", "yes", "yes", "yes"],
  },
  {
    label: "Sans compte",
    marks: ["yes", "yes", "yes", "yes", "no", "partial"],
    note: "Outplayed marche sans compte, mais avec un filigrane sur les clips.",
  },
  {
    label: "Sans publicité",
    marks: ["yes", "yes", "yes", "yes", "partial", "partial"],
    note: "Medal et Outplayed retirent les pubs dans leur version payante.",
  },
  {
    label: "Code ouvert (open source)",
    marks: ["yes", "no", "no", "no", "no", "no"],
  },
];

const LABEL: Record<Mark, string> = { yes: "Oui", no: "Non", partial: "En partie" };

function Cell({ mark }: { mark: Mark }) {
  const Icon = mark === "yes" ? Check : mark === "no" ? X : Minus;
  return (
    <span
      title={LABEL[mark]}
      className={
        mark === "yes"
          ? "text-ok inline-flex"
          : mark === "no"
            ? "text-muted inline-flex opacity-60"
            : "text-warn inline-flex"
      }
    >
      <Icon className="size-5" aria-hidden="true" />
      <span className="sr-only">{LABEL[mark]}</span>
    </span>
  );
}

/** Honest feature comparison with the usual replay tools. */
export function CompareSection() {
  const notes = ROWS.filter((r) => r.note);
  return (
    <Section id="comparer">
      <div className="grid items-end gap-6 md:grid-cols-2 md:gap-12">
        <SectionTitle>Et les autres&nbsp;?</SectionTitle>
        <p className="text-muted max-w-md text-lg">
          Tous ces outils savent garder les dernières secondes de jeu. Voici ce qui change.
        </p>
      </div>
      <div className="border-line mt-12 overflow-x-auto rounded-xl border">
        <table className="w-full min-w-[40rem] border-collapse text-sm">
          <thead>
            <tr className="border-line bg-nav border-b">
              <th scope="col" className="p-4 text-left font-medium">
                <span className="sr-only">Critère</span>
              </th>
              {TOOLS.map((t, i) => (
                <th
                  key={t}
                  scope="col"
                  className={
                    i === 0
                      ? "bg-accent/10 text-ink p-4 text-center font-semibold"
                      : "text-muted p-4 text-center font-medium"
                  }
                >
                  {t}
                </th>
              ))}
            </tr>
          </thead>
          <tbody>
            {ROWS.map((r) => (
              <tr key={r.label} className="border-line border-b last:border-b-0">
                <th scope="row" className="text-ink p-4 text-left font-medium">
                  {r.label}
                  {r.note && <span className="text-warn"> *</span>}
                </th>
                {r.marks.map((m, i) => (
                  <td
                    key={i}
                    className={i === 0 ? "bg-accent/10 p-4 text-center" : "p-4 text-center"}
                  >
                    <Cell mark={m} />
                  </td>
                ))}
              </tr>
            ))}
          </tbody>
        </table>
      </div>
      <div className="text-muted mt-4 grid gap-1 text-xs">
        {notes.map((r) => (
          <p key={r.label}>* {r.note}</p>
        ))}
        <p>
          Comparaison faite de bonne foi en novembre 2026 ; ces logiciels évoluent, vérifie sur leur
          site. ClipForge, lui, ne fait que Windows et n’a pas de partage en ligne intégré.
        </p>
      </div>
    </Section>
  );
}
