# QBeatSaberPlus-GameTweaker-Korean — Project State

Last updated: 2026-08-07

## Purpose
Quest 3 / Beat Saber 1.40.8_7379 (Scotland2)용 QBeatSaberPlus-GameTweaker를 한국어로 번역하고, 한국어 일부 글자가 다른 글자보다 작게 표시되는 TMP 폰트 문제를 해결한다.

## Repository / branch
- Repository: `anwodwlwod-spec/QBeatSaberPlus-GameTweaker-Korean`
- Active development branch: `chatgpt-font-debug`
- Main source currently being modified: `src/UI/SettingsMainView.cpp`
- Built library/artifact normally used for Quest test: `libqbeatsaberplus-gametweaker.so`

## User workflow
ChatGPT edits/commits GitHub directly. User should not normally need to copy/paste source code.
After a successful NDK build, user downloads the `libqbeatsaberplus-gametweaker.so` artifact, applies it to Quest, launches Beat Saber, enters GameTweaker settings, exits, and supplies logs when runtime verification is needed.
User uses `완` as shorthand meaning the relevant GitHub Actions build has completed and ChatGPT should check it.

## Previously solved issue
GameTweaker previously crashed when it was re-enabled. A range/out-of-bounds bug was fixed and repeated enable/disable testing then worked normally.

## Current font symptom
Most Korean UI text renders normally, but specific Hangul syllables such as:
- `룹`
- `콤`
- `밝`
- `홍`
- `죽`
render visibly smaller than surrounding Korean text.

The relevant TMP asset observed at runtime is `NotoSansKR-Medium SDF`.

## Runtime facts already established
Runtime dump of `NotoSansKR-Medium SDF` showed:
- sourceFont initially `<null>`
- CharacterTable = 600
- GlyphTable = 600
- AtlasTextures = 1
- FallbackFonts = 0

Quest OS font enumeration succeeded. `GetOSInstalledFontNames()` returned about 210 names, including `Noto Sans CJK KR`.
The earlier `HasCharacter()` scan reported the target Korean characters as supported by many OS font names; this is likely influenced by Android/Unity font fallback, so those results should not be treated as proof that every named font physically contains the glyphs. However, the presence of `Noto Sans CJK KR` and the working OS-font creation path are established.

## Latest implemented test
Commit `b0d0600` (`Attach Quest Korean OS font before TMP glyph expansion`) changed the runtime test so that when `NotoSansKR-Medium SDF` has no source font it:
1. Creates Quest OS font `Noto Sans CJK KR`.
2. Assigns it to the TMP asset's `sourceFontFile`.
3. Attempts `TryAddCharacters()` for `룹 콤 밝 홍 죽`.

GitHub Actions for that commit:
- NDK build #57: SUCCESS
- Font API diagnostic #7: SUCCESS

## Result from Quest runtime test of b0d0600 / NDK #57
Visual result: target characters are STILL small.

Paperlog established:
- `[KoreanFontExtend] Found NotoSansKR-Medium SDF | sourceFont=<null>`
- `[KoreanFontExtend] Creating Quest OS source font: Noto Sans CJK KR`
- `[KoreanFontExtend] Attached source font=Noto Sans CJK KR`
- `[KoreanFontExtend] Trying to add: 룹 콤 밝 홍 죽`
- `TryAddCharacters returned false.`
- Each target character remained `nowDirect=false`.

Therefore source font attachment itself works, but attaching `sourceFontFile` alone is insufficient to make the existing TMP asset accept new glyphs.

There were also beatsaber-hook parameter-match messages around the attempt (wrong parameter counts), so overload/signature details should be kept in mind when interpreting calls. Nevertheless the explicit runtime result is that the attempted glyph addition returned false and no target glyph became directly available.

## Current diagnosis
The existing `NotoSansKR-Medium SDF` appears to remain effectively static after source-font attachment. The next isolated test should change its TMP `atlasPopulationMode` to Dynamic and then retry `TryAddCharacters("룹콤밝홍죽")`.

Do NOT jump to unrelated font replacement approaches before testing this, because we deliberately isolated source attachment first and confirmed it works.

## Immediate next step
Modify `src/UI/SettingsMainView.cpp` on `chatgpt-font-debug` to:
1. Find `NotoSansKR-Medium SDF` as before.
2. Ensure `Noto Sans CJK KR` is attached as `sourceFontFile`.
3. Log current `atlasPopulationMode` if practical.
4. Set `atlasPopulationMode` to TMP Dynamic mode using the generated Quest/IL2CPP API available in this project.
5. Retry `TryAddCharacters` for `룹콤밝홍죽`.
6. Log return value and direct availability of each target glyph after the call.
7. Build with the existing normal `NDK build` workflow; do not create another standalone OS-font workflow.

## Workflow cleanup/history
A temporary standalone workflow `.github/workflows/os-font-runtime-diagnostic.yml` was created during diagnosis but caused confusing failed-X status and was later deleted in commit `af5b2f1`. Do not recreate it. Use the normal existing NDK build workflow.

## Important cautions
- Do not assume a successful GitHub Actions build means the runtime font fix worked; verify on Quest.
- Do not replace or globally modify unrelated Beat Saber/BSML fonts unless specifically testing that approach; the goal is to avoid affecting other mods.
- Preserve the Korean translations already present in the project.
- Prefer one-variable-at-a-time runtime tests so failures can be attributed correctly.

## Handoff instruction for a new ChatGPT chat
Read this file first, then inspect the current `chatgpt-font-debug` branch and the latest relevant commits before making changes. Continue from **Immediate next step** rather than restarting font diagnosis from scratch.
