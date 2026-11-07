# ClipForge design

## Colors

Dark theme. Primary is blue/violet.

| Role | OKLCH source | Qt sRGB |
|---|---|---|
| Canvas | oklch(0.165 0.012 275) | #101119 |
| Navigation | oklch(0.19 0.014 275) | #171822 |
| Surface | oklch(0.23 0.018 275) | #20222f |
| Primary | oklch(0.66 0.17 277) | #8c8cff |
| Text | oklch(0.96 0.006 275) | #f0f1fa |
| Secondary | oklch(0.76 0.02 275) | #b0b3c8 |

## Typography

- Text: Segoe UI Variable
- Times and technical values: Consolas
- Body: 14px
- Utility text: 12px
- Page titles: 28px

## Spacing and shape

- Spacing unit: 8px
- Corner radius: 8–12px

## Layout

- Fixed navigation
- Flexible library
- Large video workspace

## Components

- Replay ruler: horizontal bar showing the actual buffer coverage and the F8 save action. No fabricated thumbnails or metrics.
- Cards: used only for real video thumbnails.
- Selected controls: tinted surface.
- Focus: visible border.

## Motion

- State transitions: 150ms
- Disabled when reduced motion is on
