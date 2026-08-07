#pragma once

#include_next <TMPro/TMP_FontAsset.hpp>

// Compatibility shim for the bs-cordl enum wrapper used by this project.
#define AtlasPopulationMode_Unwrapped __AtlasPopulationMode_Unwrapped

// Font-debug experiment:
// Test #67 proved that UnityEngine::Font can be created successfully from
// Quest's /system/fonts/NotoSansCJK-Regular.ttc, but TMP_FontAsset::CreateFontAsset
// still throws because the generated wrapper selects the wrong overload.
//
// For test #68, skip creation of a second TMP asset entirely. Feed the
// file-backed Unity Font directly into the already loaded NotoSansKR-Medium SDF.
// The existing TryExtendLoadedKoreanFont() code will then switch that asset to
// Dynamic and call TryAddCharacters(), so success is simply 600 -> 606.
//
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
