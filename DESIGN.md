# ClipForge design

A gamer checks a saved moment in a dim room while the game keeps running: use a quiet dark surface and readable controls.
Restrained color strategy. The brief's blue/violet identity takes precedence over the randomly generated green seed.

| Role | OKLCH source | Qt sRGB |
|---|---|---|
| Canvas | oklch(0.165 0.012 275) | #101119 |
| Navigation | oklch(0.19 0.014 275) | #171822 |
| Surface | oklch(0.23 0.018 275) | #20222f |
| Primary | oklch(0.66 0.17 277) | #8c8cff |
| Text | oklch(0.96 0.006 275) | #f0f1fa |
| Secondary | oklch(0.76 0.02 275) | #b0b3c8 |

Segoe UI Variable for text, Consolas for times and technical values. 14px body, 12px utility, 28px page titles. 8px spacing unit; 8–12px corners.
Signature: a horizontal replay ruler shows the actual elapsed buffer coverage and the F8 save action. No fabricated thumbnails or metrics.
Fixed navigation, flexible library, large video workspace. Cards reserved for real video thumbnails. Selected controls use a tinted surface; focus uses a visible border. State transitions at 150ms, optional and disabled with reduced motion.
