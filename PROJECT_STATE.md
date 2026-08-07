# QBeatSaberPlus-GameTweaker-Korean — Project State

Last updated: 2026-08-07

## Purpose
Quest 3 / Beat Saber 1.40.8_7379 (Scotland2)용 QBeatSaberPlus-GameTweaker를 한국어로 번역하고, 한국어 일부 글자가 다른 글자보다 작게 표시되는 TMP 폰트 문제를 해결한다.

이 파일은 단순 기술 메모가 아니라 **새 ChatGPT 채팅으로 넘어가도 기존 작업 방식과 대화 맥락까지 최대한 유지하기 위한 handoff 파일**이다. 새 채팅에서는 이 파일만 믿고 코드를 추측하지 말고, 반드시 실제 브랜치의 최신 코드/커밋/Actions도 함께 확인한다.

## Repository / branch
- Repository: `anwodwlwod-spec/QBeatSaberPlus-GameTweaker-Korean`
- 사용자명 철자 주의: `anwodwlwod`가 맞고 `anwoodwlwod`가 아님.
- Active development branch: `chatgpt-font-debug`
- Main source currently being modified: `src/UI/SettingsMainView.cpp`
- Built library/artifact normally used for Quest test: `libqbeatsaberplus-gametweaker.so`
- PR #1은 현재 font-debug 작업을 Actions에서 추적하기 위해 열어둔 상태다.

## Conversation / collaboration conventions
- 기본 대화 언어: 한국어.
- 사용자는 기술적으로 다음 단계가 명확할 때 긴 확인 질문보다 바로 진행하는 것을 선호한다.
- `ㄱ` = 다음 단계 바로 진행.
- `완` = 관련 GitHub Actions 빌드가 끝났으니 ChatGPT가 직접 성공/실패를 확인.
- `확인`도 Actions/상태 확인 요청으로 자주 사용됨.
- ChatGPT가 GitHub에 직접 접근 가능한 상태이므로, 가능한 경우 **ChatGPT가 직접 코드 수정/커밋**한다. 사용자가 소스코드를 수동 복붙하게 하지 않는다. 실제 쓰기 권한/도구 제약으로 불가능할 때만 사용자 작업을 요청한다.
- 사용자는 Quest 쪽 설치/실행/육안 확인 및 런타임 로그 제공을 담당한다.
- 한 번에 여러 원인을 건드리기보다 **변수 하나씩 분리해서 테스트**하는 방식으로 진행한다.
- 확정된 사실 / 현재 가설을 구분한다. 빌드 성공만으로 런타임 수정 성공으로 간주하지 않는다.

## Normal test loop
1. ChatGPT가 `chatgpt-font-debug` 최신 코드를 읽는다.
2. ChatGPT가 필요한 최소 수정만 직접 커밋한다.
3. 기존 정상 `NDK build` workflow를 사용한다.
4. 사용자가 `완`이라고 보내면 ChatGPT가 Actions를 직접 확인한다.
5. 성공하면 사용자가 최신 `libqbeatsaberplus-gametweaker.so` artifact를 다운로드하여 Quest에 적용한다.
6. Beat Saber 실행 -> GameTweaker 설정 진입 -> 필요하면 설정에서 나옴/게임 종료.
7. 글자/크래시 여부를 육안 확인한다.
8. 필요 시 Paperlog / beatsaber-hook / CustomTypes 로그를 업로드한다.
9. ChatGPT가 로그를 근거로 다음 한 단계만 정한다.

## Target environment
- Meta Quest 3
- Beat Saber `1.40.8_7379`
- Scotland2 modloader
- QBeatSaberPlus-GameTweaker Korean fork
- Normal build path: existing GitHub Actions `NDK build`

## Broader project history
이 작업은 단순 폰트 실험부터 시작한 것이 아니다.

GameTweaker를 한국어화하는 과정에서 한때 **모드 비활성화는 되지만 다시 활성화할 때 크래시**가 발생했다. 여러 진단과 수정 끝에 원인이 범위/range를 벗어나는 문제로 정리되었고, 수정 후 반복적으로 비활성화/재활성화를 해도 정상 동작하는 것을 확인했다. 그 뒤 현재의 폰트 품질 문제로 넘어왔다.

따라서 현재 폰트 수정 과정에서도 이전의 크래시 안정성을 깨지 않는 것이 중요하다. `OnSettingChanged()`를 UI 생성/Refresh 직후 무조건 재호출하던 부분도 과거 진단 과정에서 피하도록 조정되어 있으므로, 관련 코드를 함부로 되돌리지 않는다.

## Current font symptom
Most Korean UI text renders normally, but specific Hangul syllables such as:
- `룹`
- `콤`
- `밝`
- `홍`
- `죽`
render visibly smaller than surrounding Korean text.

실제 번역 문자열 예시에는 `그룹`, `콤보`, `밝기`, `홍보`, `폭죽` 등이 포함된다. 문제는 전체 한국어가 작은 것이 아니라 **일부 음절만 주변 글자보다 작게 보이는 것**이다.

The relevant TMP asset observed at runtime is `NotoSansKR-Medium SDF`.

## Working theory
주요 가설은 다음과 같다.

`NotoSansKR-Medium SDF`가 완전한 한글 글리프 세트가 아니라 제한된 baked subset만 갖고 있고, 없는 음절은 다른 fallback/Unicode 렌더링 경로를 사용한다. 그 경로의 폰트 metrics/scale이 달라서 특정 음절만 작게 보인다.

이 가설을 검증하기 위해 loaded TMP font, direct/fallback character support, glyph source, Quest OS font 등을 단계적으로 진단해왔다.

## Runtime facts already established
Runtime dump of `NotoSansKR-Medium SDF` showed:
- sourceFont initially `<null>`
- CharacterTable = 600
- GlyphTable = 600
- AtlasTextures = 1
- FallbackFonts = 0

이 수치는 현재 asset이 약 600개 character/glyph만 baked된 제한형 TMP asset이라는 강한 근거다.

## Quest OS font investigation
Quest OS font enumeration succeeded. `GetOSInstalledFontNames()` returned about 210 names, including `Noto Sans CJK KR`.

The earlier `HasCharacter()` scan reported the target Korean characters as supported by many OS font names; this is likely influenced by Android/Unity font fallback, so those results should not be treated as proof that every named font physically contains the glyphs. However:
- Quest OS에 `Noto Sans CJK KR` 이름이 실제 존재한다.
- `CreateDynamicFontFromOSFont()` 경로는 런타임에서 실제 동작한다.
- 따라서 기존 TMP asset의 source font 후보로 사용할 수 있다는 점은 확정이다.

## Completed experiments

### 1. Temporary OS-font diagnostic workflow
`.github/workflows/os-font-runtime-diagnostic.yml`라는 별도 workflow를 한때 만들었다. 이 workflow가 실패하면서 커밋 옆에 빨간 X가 떠서, 실제 `NDK build`는 성공/진행 중인데도 전체가 실패한 것처럼 보이는 혼란이 있었다.

결국 별도 workflow는 불필요했고 runtime 진단을 코드에 넣는 방식으로 충분했다.

삭제 commit:
- `af5b2f1` — `Remove obsolete OS font diagnostic workflow`

**재생성하지 않는다.** 앞으로는 기존 정상 `NDK build`를 사용한다.

### 2. Attach Quest OS font as TMP source
Commit:
- `b0d0600` — `Attach Quest Korean OS font before TMP glyph expansion`

Actions:
- NDK build #57: SUCCESS
- Font API diagnostic #7: SUCCESS

이 commit에서 `NotoSansKR-Medium SDF`의 `sourceFontFile`이 null이면 Quest OS font `Noto Sans CJK KR`를 생성해서 source로 붙이고, 그 뒤 `TryAddCharacters()`로 `룹콤밝홍죽` 추가를 시도했다.

## Result from Quest runtime test of b0d0600 / NDK #57
Visual result: target characters are STILL small.

Paperlog established:
- `[KoreanFontExtend] Found NotoSansKR-Medium SDF | sourceFont=<null>`
- `[KoreanFontExtend] Creating Quest OS source font: Noto Sans CJK KR`
- `[KoreanFontExtend] Attached source font=Noto Sans CJK KR`
- `[KoreanFontExtend] Trying to add: 룹 콤 밝 홍 죽`
- `TryAddCharacters returned false.`
- Each target character remained `nowDirect=false`.

또한 그 시점의 font dump는:
- CharacterTable = 600
- GlyphTable = 600
- AtlasTextures = 1
- FallbackFonts = 0

따라서 **sourceFontFile 연결 자체는 성공**, 하지만 **기존 TMP asset에 새 글리프 추가는 실패**했다.

## Important runtime warning
Previous runtime logs around `TryAddCharacters` also showed beatsaber-hook method-resolution messages such as wrong parameter counts among overload candidates. 예: 3 parameters candidate / expected 2 등.

명시적인 mod 로그에서는 `TryAddCharacters returned false`가 확인되었지만, 만약 Dynamic population mode를 설정해도 계속 실패한다면 다음으로는 **현재 IL2CPP binding이 올바른 `TryAddCharacters` overload를 호출하는지** 검증해야 한다.

## Current diagnosis
The existing `NotoSansKR-Medium SDF` appears to remain effectively Static/non-dynamic after source-font attachment.

현재 가장 합리적인 다음 가설:
- source font는 붙었지만 atlas population mode가 Static이므로 `TryAddCharacters()`가 새 glyph를 atlas에 넣지 못한다.

이것은 아직 가설이며 런타임 검증 전에는 확정으로 취급하지 않는다.

## Immediate next step
Modify `src/UI/SettingsMainView.cpp` on `chatgpt-font-debug` to:
1. 최신 live file을 먼저 fetch한다. 이 문서에 있는 코드 기억만으로 재작성하지 않는다.
2. Find `NotoSansKR-Medium SDF` as before.
3. Ensure `Noto Sans CJK KR` is attached as `sourceFontFile`.
4. Log current `atlasPopulationMode` if practical.
5. Set `atlasPopulationMode` to TMP **Dynamic** mode using the generated Quest/IL2CPP API actually available in this project.
6. Retry `TryAddCharacters("룹콤밝홍죽")`.
7. Log return value and direct availability of each target glyph after the call.
8. 가능하면 CharacterTable/GlyphTable counts before/after도 로그에 남긴다.
9. Commit directly to `chatgpt-font-debug`.
10. Build with the existing normal `NDK build` workflow; do not create another standalone diagnostic workflow.

## If Dynamic mode succeeds
확인할 것:
- `TryAddCharacters=true`
- 5개 글자 direct=true
- character/glyph table count 증가 가능성
- 실제 UI에서 `룹/콤/밝/홍/죽` 크기가 주변 한글과 같아졌는지

로그 성공만으로 끝내지 말고 반드시 Quest 화면까지 확인한다.

## If Dynamic mode fails
다음 우선순위:
1. 실제 `atlasPopulationMode`가 런타임에서 Dynamic으로 바뀌었는지 확인.
2. `TryAddCharacters` IL2CPP overload/signature가 올바른지 확인.
3. 필요하면 기존 asset을 강제로 확장하는 대신 `TMP_FontAsset::CreateFontAsset(UnityEngine::Font*)`를 이용해 `Noto Sans CJK KR` 기반 새 TMP FontAsset/fallback을 만드는 경로 검토.
4. 다른 모드/전체 UI에 영향을 주는 글로벌 폰트 교체는 마지막에 검토. 목표는 GameTweaker 한글 문제만 국소적으로 해결하는 것.

## Important cautions
- Do not assume a successful GitHub Actions build means the runtime font fix worked; verify on Quest.
- Do not replace or globally modify unrelated Beat Saber/BSML fonts unless specifically testing that approach; the goal is to avoid affecting other mods.
- Preserve the Korean translations already present in the project.
- Preserve the earlier re-enable crash fix and UI lifecycle stability changes.
- Prefer one-variable-at-a-time runtime tests so failures can be attributed correctly.
- 커밋 전체 X와 실제 NDK build 실패를 구분한다. 개별 workflow run을 확인한다.

## Cross-chat handoff instruction
새 ChatGPT 채팅에서 사용자는 다음 정도만 말하면 된다:

`GameTweaker 한글화 프로젝트 이어서. chatgpt-font-debug의 PROJECT_STATE.md 읽고 계속해.`

새 채팅의 ChatGPT는 반드시:
1. `chatgpt-font-debug/PROJECT_STATE.md`를 읽는다.
2. live 최신 `src/UI/SettingsMainView.cpp`를 fetch한다.
3. PR/branch 최신 head와 필요한 경우 Actions 상태를 확인한다.
4. 이 문서의 **Immediate next step**부터 이어가되, 최신 repository/log evidence가 더 새로우면 그것을 우선한다.
5. 의미 있는 실험 결과가 나오면 이 파일도 업데이트한다.

이렇게 하면 긴 이전 채팅을 다시 열지 않아도 기술적 진행상황뿐 아니라 사용자와의 작업 방식, 약어, 실패했던 접근, 왜 현재 단계까지 왔는지까지 최대한 유지할 수 있다.
