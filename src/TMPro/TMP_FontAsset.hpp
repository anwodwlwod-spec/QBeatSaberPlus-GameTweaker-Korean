#pragma once

#include_next <TMPro/TMP_FontAsset.hpp>

// Compatibility shim for the bs-cordl enum wrapper used by this project.
#define AtlasPopulationMode_Unwrapped __AtlasPopulationMode_Unwrapped

// Font-debug experiment:
// Test #69 proved the working path:
//   Quest file-backed Noto CJK Font -> existing NotoSansKR-Medium SDF source
//   -> switch TMP asset to Dynamic -> missing Korean glyphs can be generated.
//
// Test #70 checks whether TMP can populate missing glyphs automatically when
// they are first rendered. Keep the successful source replacement, but suppress
// the explicit TryAddCharacters("룹콤밝홍죽") prefill by redirecting that call to
// an empty string. If 룹/콤/밝/홍/죽/듈 all still render correctly, the final fix
// only needs source attachment + Dynamic population mode.

// This macro must remain syntactically valid after a member access (`obj->`).
// Keep the original member call at the start of the replacement, then run the
// diagnostic block afterward. The self-reference is intentionally not expanded
// recursively by the preprocessor.
#define set_sourceFontFile(value) \
    set_sourceFontFile(value); \
    do { \
        Logger::Instance->Info(u"[KoreanDirectSource] Trying file-backed source on existing NotoSansKR asset..."); \
        ::UnityEngine::Font* __gt_source = nullptr; \
        try { \
            __gt_source = ::UnityEngine::Font::New_ctor(u"/system/fonts/NotoSansCJK-Regular.ttc"); \
            Logger::Instance->Info(__gt_source \
                ? u"[KoreanDirectSource] File-backed Unity Font created; attaching to existing asset." \
                : u"[KoreanDirectSource] File-backed Unity Font is null; keeping original source value."); \
        } catch (...) { \
            Logger::Instance->Info(u"[KoreanDirectSource] File-backed Unity Font creation threw; keeping original source value."); \
        } \
        if (__gt_source) { \
            l_KoreanFont->set_sourceFontFile(__gt_source); \
            Logger::Instance->Info(u"[KoreanDirectSource] Existing NotoSansKR source replaced with file-backed Font."); \
        } \
    } while (false)

// Test #70 only: neutralize the explicit prefill call in SettingsMainView.cpp.
// Function-like macro expansion after `obj->` remains a normal member call.
#define TryAddCharacters(...) TryAddCharacters(u"", false)
