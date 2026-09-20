# HAKUI L19 — Neeshego Manga Render Profile

L19 turns L16 identity, L17 runtime presence, and L18 embodiment into a deterministic manga presentation grammar that render backends can consume without moving presentation ownership into gameplay state.

## Canonical character render profiles

- **Agnathos / Raster Veil** — monochrome-only, white-subject-on-black composition, hard raster ink, strong scanline interference, moderate halftone, low xerox noise, portrait cuts, speed lines, and frame tearing.
- **Saelis / Synthetic Ink** — monochrome-only, clean synthetic ink, stronger halftone than scanline treatment, portrait cuts and speed lines, but no realm-tear corruption by default.
- **The Reaper / Xerox Crush** — monochrome-only, white skeletal subject against deep black, maximum edge ink, extreme xerox/grunge noise, low scanlines, portrait cuts, speed lines, and frame tearing.

`CharacterRenderProfile` is immutable canon. It defines the baseline visual language for a character; it does not own GPU shaders, textures, transforms, animation playback, camera state, combat state, or HOME persistence.

## Frame composition events

`composeMangaFrame()` deterministically derives a `MangaFrameTreatment` from canonical character identity plus one presentation event:

- `Exploration` — default world presentation.
- `Dialogue` — portrait-cut grammar when the profile permits it.
- `CombatWindup` — edge-ink pressure plus restrained speed-line buildup.
- `CombatImpact` — impact panel, full speed lines, black-field crush, short freeze-frame emphasis, and optional frame tear.
- `RealmShift` — realm-break panel with increased scanline/xerox interference and optional frame tear.
- `Overload` — maximum ink/black-field pressure with mixed scanline/xerox corruption and freeze-frame stress.

These are render directives, not gameplay triggers. Combat, dialogue, realm systems, or a future manga director choose the event; the render grammar only describes how to present it.

## Runtime access

`GameRuntime` exposes:

- `playerRenderProfile()`
- `npcRenderProfile(npcId)`
- `characterRenderProfile(instanceId)`
- `playerMangaFrame(event)`
- `characterMangaFrame(instanceId, event)`

This gives the native renderer a deterministic identity -> instance -> embodiment -> manga-frame chain without making the renderer authoritative over simulation.

## Acceptance

The existing `hakui.character_embodiment` contract now also verifies L19 render canon and event composition:

- every canonical character remains strictly B&W,
- Agnathos is scanline-dominant Raster Veil,
- Saelis is clean synthetic/halftone dominant,
- Reaper is xerox-noise dominant with no humanoid fallback,
- combat impact composes impact panels and speed-line/freeze-frame pressure,
- realm shift and overload preserve character-specific corruption grammar,
- runtime lookup remains stable across session reset.

## Boundary for L20

L19 defines the render machine's deterministic instructions. L20 may bind those directives to concrete GPU/post-process implementation and manga cinematography while preserving this authority boundary.
