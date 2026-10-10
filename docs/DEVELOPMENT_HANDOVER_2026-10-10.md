# 개발 기록 및 인수인계 — 2026-10-10

## 프로젝트
- 저장소: `anwodwlwod-spec/QBeatSaberPlus-GameTweaker-Korean`
- 원본: https://github.com/hardcpp/QBeatSaberPlus-GameTweaker (HardCPP)
- 목표: Quest용 Beat Saber GameTweaker의 한국어 UI 제공, 안정성 확보, 일부 한글 음절 크기 불균형 해결
- 대화 중 테스트 환경: Beat Saber 1.40.8_7379, GameTweaker 6.4.1, Meta Quest, MBF/Scotland2, GitHub Actions를 이용한 ARM64 .so 빌드

## 완료: 한국어 UI
- 여러 GameTweaker UI 문자열을 한국어로 번역.
- 소스 변경 후 GitHub Actions로 `libqbeatsaberplus-gametweaker.so` 빌드하고 Quest에 적용.
- 공식 원본 .so로 바꾸면 UI가 영어로 돌아가는 것도 확인하여 실행 중인 파일 교체 여부를 검증함.
- 번역 적용 범위 전체와 최신 변경 상태는 현재 저장소의 소스가 기준이며, 이 문서는 변경 내역 전체를 대체하지 않음.

## 완료: OFF → ON 재활성화 크래시
### 증상
- GameTweaker를 비활성화한 뒤 다시 활성화하면 게임이 즉시 종료됨.
- 초기 재활성화에서는 약 1초 후 종료, 이후 반복에서는 즉시 종료하는 현상도 관찰.
- 공식 릴리스 .so, MBF에서 완전히 제거 후 재설치, 다른 선택 모드를 제거한 최소 환경에서도 재현됨.
- Multiplayer+ 활성화/비활성화는 별도 테스트에서 정상 작동했음.
- 로그 일부에는 명확한 네이티브 크래시 스택이 없어 소스 진단 로그와 재현 테스트를 병행함.

### 수정 및 유력한 메커니즘
`src/Managers/CustomMenuLightManager.cpp`의 `CustomMenuLightManager::Init()`는 모듈 재활성화 시 다시 호출될 수 있지만 정적 상태는 프로세스 내에 남음.
- 수정본은 `m_Custom_Default`가 이미 존재하면 기존 상태를 재사용하고 `UpdateFromConfig()`만 호출한 뒤 반환함.
- 첫 초기화 전 백업 벡터(`m_DefaultPresetBackup`, `m_LevelClearedPresetBackup`, `m_LevelFailedPresetBackup`)를 비움.
- 조명 프리셋 복원 시 `std::min(백업 개수, 실제 배열 길이)`를 사용해 인덱스 범위 초과 가능성을 줄임.
- 재초기화로 중복 코루틴과 백업 중복이 발생할 수 있었고, 백업과 프리셋 배열의 크기가 다르면 범위 초과 접근 위험이 있음.
- **주의:** 정확한 크래시 명령어/스택을 확인한 것은 아니므로 '범위 초과가 유일한 직접 원인'이라고 단정하지 말 것. 수정 후 사용자가 OFF→ON 정상 작동을 확인한 것이 실증 결과임.

### 유지할 사항
- 정상 작동이 확인된 최신 .so와 해당 Git 커밋을 보존.
- 진단용으로 생성했던 `GameTweaker_full_diagnostic.cpp`, `CustomMenuLightManager_full_diagnostic.cpp`는 임시 테스트 파일이며 안정판과 혼동하지 말 것.
- 수정 전 공식 .so에서도 재현되었다는 사실을 한글 번역 코드 결함의 증거로 해석하지 말 것.

## 현재 미해결: 일부 한글 음절만 작게 표시
- 보고된 글자 예: `룹`, `콤`, `밝`, `홍`, `죽`.
- 가능성: TextMeshPro(TMP) 기본 폰트 아틀라스의 해당 음절 누락, fallback 폰트의 글리프 메트릭 차이.
- 아직 확정 진단 아님. 실제 적용 폰트/아틀라스/대체 폰트를 조사할 것.
- 목표: **GameTweaker UI에만** 한글 폰트를 적용하고 다른 모드나 게임 전체의 폰트는 변경하지 않기.
- 후보 대상: `src/UI/SettingsMainView.cpp`, `src/UI/SettingsMainView.hpp`, `src/UI/SettingsLeftView.cpp` 및 공통 UI 텍스트 생성부.
- 먼저 게임 내에 이미 로드된 한글 지원 TMP 폰트를 재사용할 수 있는지 확인. 불충분하면 전용 TMP Font Asset/AssetBundle 제작·로딩 검토.

## 빌드/패키징 확인 사항
사용자가 확인해 준 프로젝트 구성:
- `CMakeLists.txt`: C++20, `src/*.cpp` 재귀 수집, 공유 라이브러리 생성, strip 및 debug 심볼 분리.
- `qpm.json`: 프로젝트 6.4.1, NDK `^27.2.12479018`, 의존성 beatsaber-hook, bs-cordl, custom-types, scotland2, paper2_scotland2, kaleb, chatplex-sdk-bs.
- `mod.template.json` (대화에서는 파일명 `mod.template.jso`로도 기재): Scotland2, Beat Saber `1.40.8_7379`, `lateModFiles: ["${binary}"]`, `fileCopies: []`.
- 현재 패키징 템플릿에는 별도 폰트 AssetBundle 배포 항목이 없음. 폰트 파일을 단순히 저장소에 추가하는 것만으로 Quest에 설치되지는 않음.
- GitHub Actions로 C++/NDK 빌드하므로 로컬 PC에 CMake/QPM/NDK를 직접 설치할 필요는 없음(기존 CI가 정상 작동한다는 전제).
- 새 TMP Font AssetBundle을 생성하려면 일반적으로 호환 Unity 제작 환경이 필요하지만, **사용자 PC에 반드시 Unity를 설치해야 한다는 뜻은 아님**. 사전 제작 번들 또는 별도 빌드 환경 활용 가능.

## 다음 작업 절차
1. 안정판 커밋과 정상 작동 .so를 보존하고 변경은 별도 커밋으로 관리.
2. 현재 GameTweaker UI 텍스트 생성 경로와 실제 TMP 폰트/fallback 선택을 진단.
3. 문제 음절을 포함한 글리프 지원 여부 및 글꼴 메트릭 확인.
4. 전용 폰트 적용 범위를 GameTweaker UI로 제한.
5. 필요 시 AssetBundle 생성 및 `mod.template.json` 패키징/설치 경로 추가.
6. GitHub Actions 빌드 → Quest 테스트(한글 크기, 메뉴 재진입, OFF→ON 반복) → 로그 확인.
7. 재활성화 크래시 수정이 회귀하지 않는지 반드시 확인.

## 협업 운영 메모
- GitHub 웹 편집에서 두 파일을 각각 커밋하면 Actions 빌드도 두 번 실행될 수 있음.
- **마지막 두 변경 사항이 모두 포함된 최신 성공 빌드**의 .so를 사용해야 함.
- .so 파일과 소스 .cpp를 혼동하지 말 것. C++ 소스는 저장소에 커밋하고 Actions에서 빌드된 .so만 Quest의 실제 활성 모드 위치에 배포.
- 크래시 진단은 `Paperlog.log`, `beatsaber-hook.log`, 필요하면 Android tombstone/스택을 함께 확인.
- 로그 문자열 `[GT-FULL-DIAG]`, `[GT-LIGHT-DIAG]`, `[GT-LIGHT-FIX]`는 각 테스트 버전에서 사용되었음.

이 문서는 채팅에서 확인된 사실과 합리적인 기술적 가설을 구분하여 기록한 인수인계 문서다.
