#pragma once

#include_next <TMPro/TMP_FontAsset.hpp>

// Compatibility shim for the bs-cordl enum wrapper used by this project.
#define AtlasPopulationMode_Unwrapped __AtlasPopulationMode_Unwrapped

// Font-debug experiment:
// The bundled NotoSansKR-Medium SDF is a static asset with no source font.
// When SettingsMainView attaches Quest's OS font as its source, also create a
// fresh TMP_FontAsset from that same UnityEngine::Font and append it as a
// fallback of the bundled Korean asset. Teko already resolves ordinary Korean
// through NotoSansKR, so missing syllables can then continue into this fresh
// dynamic asset without replacing Teko or changing Latin text rendering.
//
// This macro intentionally relies on the local variable name l_KoreanFont in
// TryExtendLoadedKoreanFont(); it is isolated to this diagnostic branch/TU.
#define set_sourceFontFile(value) \
    set_sourceFontFile(value); \
    do { \
        Logger::Instance->Info(u"[KoreanDynamicFallback] Creating fresh TMP font asset..."); \
        auto* __gt_source = static_cast<::UnityEngine::Font*>(value); \
        auto __gt_dynamic = ::TMPro::TMP_FontAsset::CreateFontAsset(__gt_source); \
        if (__gt_dynamic) { \
            Logger::Instance->Info(u"[KoreanDynamicFallback] CreateFontAsset succeeded."); \
            __gt_dynamic->set_name(u"GameTweaker-Korean-Dynamic"); \
            __gt_dynamic->set_atlasPopulationMode(::TMPro::AtlasPopulationMode(::TMPro::AtlasPopulationMode::Dynamic)); \
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
            Logger::Instance->Info(u"[KoreanDynamicFallback] CreateFontAsset returned null."); \
        } \
    } while (false)
