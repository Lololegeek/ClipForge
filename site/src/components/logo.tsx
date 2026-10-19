export function Logo({ size = 30 }: { size?: number }) {
  return (
    <svg width={size} height={size} viewBox="0 0 48 48" aria-hidden="true" className="shrink-0">
      <rect x="1" y="1" width="46" height="46" rx="11" fill="#8c8cff" />
      <g
        fill="none"
        stroke="#101119"
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth="3.5"
      >
        <path d="M20 13h-7v7M28 13h7v7M13 28v7h7M35 28v7h-7" />
        <path d="m19 30 11-11" strokeWidth="4.5" />
      </g>
    </svg>
  );
}
