#pragma once

#include_next <TMPro/TMP_FontAsset.hpp>

// Compatibility shim for the bs-cordl enum wrapper used by this project.
#define AtlasPopulationMode_Unwrapped __AtlasPopulationMode_Unwrapped

// Font-debug experiment:
// NDK #66 confirmed that TMP_FontAsset::CreateFontAsset() still returns null
// when its source is the Unity Font created by CreateDynamicFontFromOSFont().
// Unity's TMP guidance recommends using a file-backed Font for runtime asset
// creation, so try Quest/Android's CJK font file directly first.
//
// This macro intentionally relies on the local variable name l_KoreanFont in
// TryExtendLoadedKoreanFont(); it is isolated to this diagnostic branch/TU.
#define set_sourceFontFile(value) \
    set_sourceFontFile(value); \
    do { \
        Logger::Instance->Info(u"[KoreanDynamicFallback] Trying file-backed Quest CJK font..."); \
        ::UnityEngine::Font* __gt_source = nullptr; \
        try { \
            __gt_source = ::UnityEngine::Font::New_ctor(u"/system/fonts/NotoSansCJK-Regular.ttc"); \
            Logger::Instance->Info(__gt_source \
                ? u"[KoreanDynamicFallback] File-backed Unity Font created." \
                : u"[KoreanDynamicFallback] File-backed Unity Font is null."); \
        } catch (...) { \
            Logger::Instance->Info(u"[KoreanDynamicFallback] File-backed Unity Font creation threw."); \
        } \
        if (!__gt_source) \
            __gt_source = static_cast<::UnityEngine::Font*>(value); \
        auto __gt_dynamic = ::TMPro::TMP_FontAsset::CreateFontAsset( \
            __gt_source, \
            90, \
            l_KoreanFont->get_atlasPadding(), \
            l_KoreanFont->get_atlasRenderMode(), \
            l_KoreanFont->get_atlasWidth(), \
            l_KoreanFont->get_atlasHeight(), \
            ::TMPro::AtlasPopulationMode(::TMPro::AtlasPopulationMode::Dynamic), \
            true); \
        if (__gt_dynamic) { \
            Logger::Instance->Info(u"[KoreanDynamicFallback] File-backed CreateFontAsset succeeded."); \
            __gt_dynamic->set_name(u"GameTweaker-Korean-Dynamic"); \
            __gt_dynamic->set_isMultiAtlasTexturesEnabled(true); \
            const bool __gt_added = __gt_dynamic->TryAddCharacters(u"룹콤밝홍죽듈", false); \
            Logger::Instance->Info(__gt_added \
                ? u"[KoreanDynamicFallback] TryAddCharacters(룹콤밝홍죽듈)=true" \
                : u"[KoreanDynamicFallback] TryAddCharacters(룹콤밝홍죽듈)=false"); \
            auto* __gt_fallbacks = l_KoreanFont->get_fallbackFontAssetTable(); \
            if (__gt_fallbacks) { \
                __gt_fallbacks->Add(__gt_dynamic); \
                Logger::Instance->Info(u"[KoreanDynamicFallback] Added to NotoSansKR fallback table."); \
            } else { \
                Logger::Instance->Info(u"[KoreanDynamicFallback] NotoSansKR fallback table is null."); \
            } \
        } else { \
            Logger::Instance->Info(u"[KoreanDynamicFallback] File-backed CreateFontAsset returned null."); \
        } \
    } while (false)
