# ClipForge

## Register

product

## Purpose

Windows desktop app. It keeps a local replay buffer, saves clips on F8, and provides a library to find, trim and export them. Offline only: no account, no cloud.

## Users

Windows gamers who save a moment with F8 without leaving the game or creating an account.

## Goals

- Keep a local replay buffer running.
- Save playable clips.
- Find clips in a library.
- Trim and export clips offline.

## Interface

French UI. Short labels. The capture state is always visible.

## Out of scope

- Web dashboard
- Social feed
- Advertising
- Invented statistics
- Default Qt widget look
- Reproduction of Medal artwork

## Principles

- Capture reliability comes first.
- One clear save action (F8).
- Original videos stay accessible.
- Fallbacks are explained to the user.
- Unsupported operations are disabled explicitly.

## Accessibility

- Keyboard focus
- Readable contrast
- Windows DPI scaling
- States shown as text as well as color
- Limited motion
