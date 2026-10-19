/* eslint-disable @typescript-eslint/no-unused-vars -- `node` est retiré des props passées au DOM */
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";

/** Rend un fichier Markdown du dépôt. Les tableaux défilent dans leur propre conteneur. */
export function Markdown({ children }: { children: string }) {
  return (
    <div className="prose-cf">
      <ReactMarkdown
        remarkPlugins={[remarkGfm]}
        components={{
          table: ({ node: _node, ...props }) => (
            <div className="border-line bg-surface/40 my-8 overflow-x-auto rounded-xl border">
              <table {...props} />
            </div>
          ),
          a: ({ node: _node, href, ...props }) => (
            <a
              href={href}
              {...(href?.startsWith("http")
                ? { target: "_blank", rel: "noopener noreferrer" }
                : {})}
              {...props}
            />
          ),
        }}
      >
        {children}
      </ReactMarkdown>
    </div>
  );
}
