"use client";

import { motion } from "motion/react";
import { useInView } from "react-intersection-observer";
import { cn } from "@/lib/utils";

/** Légère entrée au défilement : seul le décalage est animé, le contenu reste toujours lisible. */
export function Reveal({
  children,
  className,
  delay = 0,
}: {
  children: React.ReactNode;
  className?: string;
  delay?: number;
}) {
  const { ref, inView } = useInView({ triggerOnce: true, rootMargin: "0px 0px -8% 0px" });
  return (
    <motion.div
      ref={ref}
      className={cn(className)}
      initial={{ y: 18 }}
      animate={inView ? { y: 0 } : undefined}
      transition={{ duration: 0.5, delay, ease: [0.2, 0.8, 0.2, 1] }}
    >
      {children}
    </motion.div>
  );
}
