#include "UI/SettingsMainView.hpp"
#include "Managers/CustomMenuLightManager.hpp"
#include "GameTweaker.hpp"
#include "GTConfig.hpp"
#include "Logger.hpp"

#include <CP_SDK/UI/ValueFormatters.hpp>
#include <CP_SDK/Unity/Extensions/ColorU.hpp>

#include <TMPro/TMP_FontAsset.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Font.hpp>

using namespace CP_SDK::Unity::Extensions;
using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker::UI {

    /// @brief Log loaded TMP fonts that can render the Korean glyphs that were
    ///        visually smaller in the translated UI. This is diagnostic only:
    ///        it does not change any font or affect another mod.
    static void LogKoreanFontCandidates()
    {
        static bool s_HasRun = false;
        if (s_HasRun)
            return;
        s_HasRun = true;

        try
        {
            auto l_Fonts = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>();
            Logger::Instance->Info(u"[KoreanFontDiag] Scanning loaded TMP font assets...");

            int l_Total = 0;
            int l_Matches = 0;
            for (auto& l_Font : l_Fonts)
            {
                if (!l_Font)
                    continue;

                ++l_Total;
                const bool l_Group = l_Font->HasCharacter(u'룹', false, false);
                const bool l_Combo = l_Font->HasCharacter(u'콤', false, false);
                const bool l_Bright = l_Font->HasCharacter(u'밝', false, false);
                const bool l_Promo = l_Font->HasCharacter(u'홍', false, false);
                const bool l_Firework = l_Font->HasCharacter(u'죽', false, false);

                if (!(l_Group && l_Combo && l_Bright && l_Promo && l_Firework))
                    continue;

                ++l_Matches;
                std::u16string l_Message = u"[KoreanFontDiag] Candidate: ";
                l_Message += l_Font->get_name();
                Logger::Instance->Info(l_Message);
            }

            if (l_Matches == 0)
                Logger::Instance->Info(u"[KoreanFontDiag] Scan complete: no font supports all test glyphs.");
            else
                Logger::Instance->Info(u"[KoreanFontDiag] Scan complete: one or more candidates found above.");
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanFontDiag] Font scan failed:");
            Logger::Instance->Error(l_Exception);
        }
    }

    /// @brief Compare direct glyph support with TMP fallback-chain support.
    ///        This preserves the translated UI and all prior crash fixes.
    static void LogKoreanGlyphResolution()
    {
        static bool s_HasRun = false;
        if (s_HasRun)
            return;
        s_HasRun = true;

        try
        {
            auto l_Texts = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_Text*>();
            TMPro::TMP_FontAsset* l_Font = nullptr;

            for (auto& l_Text : l_Texts)
            {
                if (!l_Text)
                    continue;

                const std::u16string l_Value = l_Text->get_text();
                if (l_Value.find(u"환경") == std::u16string::npos
                    && l_Value.find(u"그룹") == std::u16string::npos
                    && l_Value.find(u"콤보") == std::u16string::npos
                    && l_Value.find(u"밝기") == std::u16string::npos
                    && l_Value.find(u"홍보") == std::u16string::npos
                    && l_Value.find(u"폭죽") == std::u16string::npos)
                    continue;

                auto l_CurrentFont = l_Text->get_font();
                if (l_CurrentFont)
                {
                    l_Font = l_CurrentFont;
                    break;
                }
            }

            if (!l_Font)
            {
                Logger::Instance->Info(u"[KoreanGlyphDiag] Could not find the GameTweaker TMP font.");
                return;
            }

            std::u16string l_FontMessage = u"[KoreanGlyphDiag] Base font: ";
            l_FontMessage += l_Font->get_name();
            Logger::Instance->Info(l_FontMessage);

            const char16_t l_Chars[] = {
                u'환', u'경', u'음', u'악', u'제', u'거',
                u'룹', u'콤', u'밝', u'홍', u'죽'
            };

            for (char16_t l_Char : l_Chars)
            {
                const bool l_Direct = l_Font->HasCharacter(l_Char, false, false);
                const bool l_WithFallback = l_Font->HasCharacter(l_Char, true, false);

                std::u16string l_Message = u"[KoreanGlyphDiag] ";
                l_Message += l_Char;
                l_Message += u" | direct=";
                l_Message += l_Direct ? u"true" : u"false";
                l_Message += u" | fallback=";
                l_Message += l_WithFallback ? u"true" : u"false";
                Logger::Instance->Info(l_Message);
            }

            Logger::Instance->Info(u"[KoreanGlyphDiag] Scan complete.");
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanGlyphDiag] Scan failed:");
            Logger::Instance->Error(l_Exception);
        }
    }

    /// @brief List every loaded TMP font asset that directly contains each test glyph.
    ///        This identifies the real loaded source font without relying on
    ///        TMP_FontAssetUtilities overload signatures.
    static void LogKoreanGlyphSourceFonts()
    {
        static bool s_HasRun = false;
        if (s_HasRun)
            return;
        s_HasRun = true;

        try
        {
            auto l_Fonts = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>();

            const char16_t l_Chars[] = {
                u'환', u'경', u'음', u'악', u'제', u'거',
                u'룹', u'콤', u'밝', u'홍', u'죽'
            };

            Logger::Instance->Info(u"[KoreanGlyphSource] Scanning loaded TMP font assets per glyph...");

            for (char16_t l_Char : l_Chars)
            {
                bool l_Found = false;

                for (auto& l_Font : l_Fonts)
                {
                    if (!l_Font)
                        continue;

                    if (!l_Font->HasCharacter(l_Char, false, false))
                        continue;

                    l_Found = true;

                    std::u16string l_Message = u"[KoreanGlyphSource] ";
                    l_Message += l_Char;
                    l_Message += u" -> ";
                    l_Message += l_Font->get_name();
                    Logger::Instance->Info(l_Message);
                }

                if (!l_Found)
                {
                    std::u16string l_Message = u"[KoreanGlyphSource] ";
                    l_Message += l_Char;
                    l_Message += u" -> <no loaded TMP font asset>";
                    Logger::Instance->Info(l_Message);
                }
            }

            Logger::Instance->Info(u"[KoreanGlyphSource] Scan complete.");
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanGlyphSource] Scan failed:");
            Logger::Instance->Error(l_Exception);
        }
    }

    static std::u16string ToU16Number(std::size_t p_Value)
    {
        const auto l_Value = std::to_string(p_Value);
        return std::u16string(l_Value.begin(), l_Value.end());
    }

    /// @brief Try to extend the already-loaded NotoSansKR TMP font at runtime
    ///        with the Korean syllables that currently fall through to unicode sprites.
    ///        No AssetBundle / CMake changes are required.
    static void TryExtendLoadedKoreanFont()
    {
        static bool s_HasRun = false;
        if (s_HasRun)
            return;
        s_HasRun = true;

        try
        {
            auto l_Fonts = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>();
            TMPro::TMP_FontAsset* l_KoreanFont = nullptr;

            for (auto& l_Font : l_Fonts)
            {
                if (!l_Font)
                    continue;

                if (l_Font->get_name() == u"NotoSansKR-Medium SDF")
                {
                    l_KoreanFont = l_Font;
                    break;
                }
            }

            if (!l_KoreanFont)
            {
                Logger::Instance->Info(u"[KoreanFontExtend] NotoSansKR-Medium SDF not found.");
                return;
            }

            auto l_SourceFont = l_KoreanFont->get_sourceFontFile();

std::u16string l_Info = u"[KoreanFontExtend] Found NotoSansKR-Medium SDF | sourceFont=";
l_Info += l_SourceFont ? l_SourceFont->get_name() : u"<null>";
Logger::Instance->Info(l_Info);

            Logger::Instance->Info(u"[NotoDump] ===== Font Details =====");

            std::u16string l_DumpInfo = u"[NotoDump] CharacterTable=";
            l_DumpInfo += ToU16Number(static_cast<std::size_t>(l_KoreanFont->get_characterTable()->get_Count()));
            Logger::Instance->Info(l_DumpInfo);

            l_DumpInfo = u"[NotoDump] GlyphTable=";
            l_DumpInfo += ToU16Number(static_cast<std::size_t>(l_KoreanFont->get_glyphTable()->get_Count()));
            Logger::Instance->Info(l_DumpInfo);

            l_DumpInfo = u"[NotoDump] AtlasTextures=";
            l_DumpInfo += ToU16Number(static_cast<std::size_t>(l_KoreanFont->get_atlasTextures().size()));
            Logger::Instance->Info(l_DumpInfo);

            auto fb = l_KoreanFont->get_fallbackFontAssetTable();

            l_DumpInfo = u"[NotoDump] FallbackFonts=";
            l_DumpInfo += ToU16Number(static_cast<std::size_t>(fb ? fb->get_Count() : 0));
            Logger::Instance->Info(l_DumpInfo);

            Logger::Instance->Info(u"[NotoDump] ===== End =====");

            if (!l_SourceFont)
            {
                Logger::Instance->Info(u"[KoreanFontExtend] Source font is null; TryAddCharacters skipped.");
                return;
            }

            Logger::Instance->Info(u"[KoreanFontExtend] Trying to add: 룹 콤 밝 홍 죽");
            const bool l_Result = l_KoreanFont->TryAddCharacters(u"룹콤밝홍죽", false);

            Logger::Instance->Info(
                l_Result
                    ? u"[KoreanFontExtend] TryAddCharacters returned true."
                    : u"[KoreanFontExtend] TryAddCharacters returned false."
            );

            const char16_t l_Chars[] = { u'룹', u'콤', u'밝', u'홍', u'죽' };

            for (char16_t l_Char : l_Chars)
            {
                std::u16string l_Message = u"[KoreanFontExtend] ";
                l_Message += l_Char;
                l_Message += u" | nowDirect=";
                l_Message += l_KoreanFont->HasCharacter(l_Char, false, false) ? u"true" : u"false";
                Logger::Instance->Info(l_Message);
            }
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanFontExtend] Failed:");
            Logger::Instance->Error(l_Exception);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    CP_SDK_IL2CPP_INHERIT_INIT(SettingsMainView);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CP_SDK_IL2CPP_DECLARE_CTOR_IMPL(SettingsMainView)
    {
        m_PreventChanges = false;

        OnViewCreation      = {this, &SettingsMainView::OnViewCreation_Impl};
        OnViewDeactivation  = {this, &SettingsMainView::OnViewDeactivation_Impl};
    }
    /// @brief Destructor
    CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_IMPL(SettingsMainView)
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On view creation
    void SettingsMainView::OnViewCreation_Impl()
    {
        /// Try to populate the existing Korean TMP font before the UI text is built.
        TryExtendLoadedKoreanFont();

        Templates::FullRectLayoutMainView({
            Templates::TitleBar(u"게임 트위커 | 설정"),

            XUITabControl::Make({
                {u"파티클",       BuildParticlesTab()},
                {u"환경",     BuildEnvironmentTab()},
                {u"곡 선택",  BuildLevelSelectionTab()},
                {u"플레이어 설정",   BuildPlayerOptionsTab()},
                {u"메인 메뉴",        BuildMainMenuTab()}
            })
            ->Bind(&m_TabControl)
            ->AsShared()
        })
        ->SetBackground(true, std::nullopt, true)
        ->BuildUI(get_transform());

        /// Diagnostics only. These calls do not change the UI font.
        LogKoreanFontCandidates();
        LogKoreanGlyphResolution();
        LogKoreanGlyphSourceFonts();

        /// Diagnostic: do not call OnSettingChanged() during view creation.
        /// This avoids re-applying patches while BS+/CP_SDK is attaching the new view.
        Logger::Instance->Info(u"[GameTweaker-UI-Diag] Skipped OnSettingChanged during view creation");
    }
    /// @brief On view deactivation
    void SettingsMainView::OnViewDeactivation_Impl()
    {
        GTConfig::Instance()->Save();
        Managers::CustomMenuLightManager::SwitchToBase();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Build particles tab
    std::shared_ptr<IXUIElement> SettingsMainView::BuildParticlesTab()
    {
        auto l_Config = GTConfig::Instance();

        return XUIVLayout::Make({
            XUIText::Make(u"모든 노트 파편 생성 제거")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->RemoveDebris)->Bind(&m_ParticlesTab_RemoveDebris)->OnValueChanged([this](bool) { OnSettingChanged(); })->AsShared(),

            XUIHLayout::Make({
                XUIVLayout::Make({
                    XUIText::Make(u"환경 파티클 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveWorldParticles)->Bind(&m_ParticlesTab_RemoveWorldParticles)->AsShared(),
                    XUIText::Make(u"바닥 화염 파티클 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveSaberBurnMarkSparkles)->Bind(&m_ParticlesTab_RemoveFloorBurnMarkParticles)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) { x->OnValueChanged([this](bool) { OnSettingChanged(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"노트 절단 파티클 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveAllCutParticles)->Bind(&m_ParticlesTab_RemoveCutParticles)->AsShared(),
                    XUIText::Make(u"바닥 베기 자국 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveSaberBurnMarks)->Bind(&m_ParticlesTab_RemoveFloorBurnMarkEffects)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) { x->OnValueChanged([this](bool) { OnSettingChanged(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"장애물 파티클 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveObstacleParticles)->Bind(&m_ParticlesTab_RemoveObstacleParticles)->AsShared(),
                    XUIText::Make(u"세이버 충돌 효과 제거")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveSaberClashEffects)->Bind(&m_ParticlesTab_RemoveSaberClashEffects)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) { x->OnValueChanged([this](bool) { OnSettingChanged(); }); })
                ->AsShared()
            })
            ->SetPadding(0)
            ->AsShared()
        })->AsShared();
    }
    /// @brief Build environment tab
    std::shared_ptr<IXUIElement> SettingsMainView::BuildEnvironmentTab()
    {
        auto& l_Config = GTConfig::Instance()->Environment;
        return XUIVLayout::Make({
            XUIText::Make(u"환경의 음악 그룹 로고 제거 (BTS, Linkin Park 등)")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->RemoveMusicBandLogo)->Bind(&m_EnvironmentTab_RemoveMusicBandLogo)->AsShared(),

            XUIText::Make(u"풀 콤보 실패 애니메이션 제거")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->RemoveFullComboLossAnimation)->Bind(&m_EnvironmentTab_RemoveFullComboLossAnimation)->AsShared()
        })
        ->ForEachDirect<XUIToggle>([this](XUIToggle* x) -> void {
            x->OnValueChanged([this](bool) -> void { OnSettingChanged(); });
        })
        ->AsShared();
    }
    /// @brief Build level selection tab
    std::shared_ptr<IXUIElement> SettingsMainView::BuildLevelSelectionTab()
    {
        auto& l_Config = GTConfig::Instance()->LevelSelection;

        return XUIVLayout::Make({
            XUIHLayout::Make({
                XUIVLayout::Make({
                    XUIText::Make(u"플레이한 곡 색상")->AsShared(),
                    XUIColorInput::Make()->SetValue(ColorU::Convert(l_Config->HighlightPlayed))->Bind(&m_LevelSelectionTab_HighlightPlayedSongColor)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color) { OnSettingChanged(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"플레이한 곡 강조")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->HighlightEnabled)->Bind(&m_LevelSelectionTab_HighlightPlayedSong)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIToggle>([this](XUIToggle* x) { x->OnValueChanged([this](bool) { OnSettingChanged(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"모든 난이도 플레이 완료 색상")->AsShared(),
                    XUIColorInput::Make()->SetValue(ColorU::Convert(l_Config->HighlightAllPlayed))->Bind(&m_LevelSelectionTab_HighlightPlayedSongAllColor)->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color) { OnSettingChanged(); }); })
                ->AsShared()
            })
            ->SetPadding(0)
            ->AsShared(),

            XUIText::Make(u"기본 게임 필터 제거")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->RemoveBaseGameFilterButton)->Bind(&m_LevelSelectionTab_RemoveBaseGameFilterButton)->AsShared()
        })
        ->ForEachDirect<XUIToggle>([this](XUIToggle* x) { x->OnValueChanged([this](bool) { OnSettingChanged(); }); })
        ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color) { OnSettingChanged(); }); })
        ->AsShared();
    }
    /// @brief Build player options tab
    std::shared_ptr<IXUIElement> SettingsMainView::BuildPlayerOptionsTab()
    {
        auto& l_Config = GTConfig::Instance()->PlayerOptions;
        return XUIVLayout::Make({
            XUIText::Make(u"플레이어 설정 메뉴 개선")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->ReorderPlayerSettings)->Bind(&m_PlayerOptionsTab_ReorderPlayerSettings)->AsShared(),

            XUIText::Make(u"조명 밝기 재정의 옵션 추가")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->OverrideLightIntensityOption)->Bind(&m_PlayerOptionsTab_AddOverrideLightIntensityOption)->AsShared(),

            XUIText::Make(u"조명 효과 필터 옵션 통합")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->MergeLightPressetOptions)->Bind(&m_PlayerOptionsTab_MergeLightEffectFilterOptions)->AsShared()

        })
        ->ForEachDirect<XUIToggle>([this](XUIToggle* x) -> void {
            x->OnValueChanged([this](bool) -> void { OnSettingChanged(); });
        })
        ->AsShared();
    }
    /// @brief Build main menu tab
    std::shared_ptr<IXUIElement> SettingsMainView::BuildMainMenuTab()
    {
        auto& l_Config = GTConfig::Instance()->MainMenu;

        return XUIVLayout::Make({
            XUIText::Make(u"메뉴 환경 색상 재정의")->AsShared(),
            XUIToggle::Make()->SetValue(l_Config->OverrideMenuEnvColors)->Bind(&m_MainMenuTab_OverrideMenuEnvColors)->OnValueChanged([this](bool) { OnSettingChangedOverrideMenuColor(); })->AsShared(),

            XUIHLayout::Make({
                XUIVLayout::Make({
                    XUIText::Make(u"기본 색상")->AsShared(),
                    XUIColorInput::Make()->SetValue(l_Config->BaseColor)->Bind(&m_MainMenuTab_BaseColor)->AsShared(),

                    XUIText::Make(u"새 콘텐츠 홍보 제거")->SetAlign(TMPro::TextAlignmentOptions::CenterGeoAligned)->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->RemoveNewContentPromotional)->Bind(&m_MainMenuTab_RemoveNewContentPromotional)->OnValueChanged([this](bool) { OnSettingChanged(); })->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color) { OnSettingChangedOverrideMenuColor(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"레벨 성공 색상")->AsShared(),
                    XUIColorInput::Make()->SetValue(l_Config->LevelClearedColor)->Bind(&m_MainMenuTab_LevelClearedColor)->AsShared(),

                    XUIText::Make(u" ")->AsShared(),
                    XUIText::Make(u" ")->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color) { OnSettingChangedLevelClearedColor(); }); })
                ->AsShared(),

                XUIVLayout::Make({
                    XUIText::Make(u"레벨 실패 색상")->AsShared(),
                    XUIColorInput::Make()->SetValue(l_Config->LevelFailedColor)->Bind(&m_MainMenuTab_LevelFailedColor)->AsShared(),

                    XUIText::Make(u"폭죽 비활성화")->AsShared(),
                    XUIToggle::Make()->SetValue(l_Config->DisableFireworks)->Bind(&m_MainMenuTab_DisableFireworks)->OnValueChanged([this](bool) { OnSettingChanged(); })->AsShared()
                })
                ->SetPadding(0)
                ->SetWidth(40.0f)
                ->ForEachDirect<XUIColorInput>([this](XUIColorInput* x) { x->OnValueChanged([this](Color)  { OnSettingChangedLevelFailedColor(); }); })
                ->AsShared()
            })
            ->SetPadding(0)
            ->AsShared()
        })->AsShared();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief When settings are changed
    void SettingsMainView::OnSettingChanged()
    {
        if (m_PreventChanges)
            return;

        #pragma region Particles_tab
        auto l_ParticlesConfig = GTConfig::Instance();
        l_ParticlesConfig->RemoveDebris                     = m_ParticlesTab_RemoveDebris->Element()->GetValue();
        l_ParticlesConfig->RemoveAllCutParticles            = m_ParticlesTab_RemoveCutParticles->Element()->GetValue();
        l_ParticlesConfig->RemoveObstacleParticles          = m_ParticlesTab_RemoveObstacleParticles->Element()->GetValue();
        l_ParticlesConfig->RemoveSaberBurnMarkSparkles      = m_ParticlesTab_RemoveFloorBurnMarkParticles->Element()->GetValue();
        l_ParticlesConfig->RemoveSaberBurnMarks             = m_ParticlesTab_RemoveFloorBurnMarkEffects->Element()->GetValue();
        l_ParticlesConfig->RemoveSaberClashEffects          = m_ParticlesTab_RemoveSaberClashEffects->Element()->GetValue();
        l_ParticlesConfig->RemoveWorldParticles             = m_ParticlesTab_RemoveWorldParticles->Element()->GetValue();
        #pragma endregion Particles_tab

        #pragma region Environment_Tab
        auto& l_EnvironmentConfig = GTConfig::Instance()->Environment;
        l_EnvironmentConfig->RemoveMusicBandLogo           = m_EnvironmentTab_RemoveMusicBandLogo->Element()->GetValue();
        l_EnvironmentConfig->RemoveFullComboLossAnimation  = m_EnvironmentTab_RemoveFullComboLossAnimation->Element()->GetValue();
        #pragma endregion Environment_Tab

        #pragma region LevelSelection_Tab
        auto& l_LevelSelectionConfig = GTConfig::Instance()->LevelSelection;
        l_LevelSelectionConfig->RemoveBaseGameFilterButton = m_LevelSelectionTab_RemoveBaseGameFilterButton->Element()->GetValue();
        l_LevelSelectionConfig->HighlightEnabled           = m_LevelSelectionTab_HighlightPlayedSong->Element()->GetValue();
        l_LevelSelectionConfig->HighlightPlayed            = ColorU::Convert(m_LevelSelectionTab_HighlightPlayedSongColor->Element()->GetValue());
        l_LevelSelectionConfig->HighlightAllPlayed         = ColorU::Convert(m_LevelSelectionTab_HighlightPlayedSongAllColor->Element()->GetValue());
        #pragma endregion LevelSelection_Tab

        #pragma region PlayerOptions_Tab
        auto& l_PlayerOptionConfig = GTConfig::Instance()->PlayerOptions;
        l_PlayerOptionConfig->ReorderPlayerSettings        = m_PlayerOptionsTab_ReorderPlayerSettings->Element()->GetValue();
        l_PlayerOptionConfig->OverrideLightIntensityOption = m_PlayerOptionsTab_AddOverrideLightIntensityOption->Element()->GetValue();
        l_PlayerOptionConfig->MergeLightPressetOptions     = m_PlayerOptionsTab_MergeLightEffectFilterOptions->Element()->GetValue();
        #pragma endregion PlayerOptions_Tab

        #pragma region MainMenu_Tab
        auto& l_MainMenuConfig = GTConfig::Instance()->MainMenu;
        l_MainMenuConfig->OverrideMenuEnvColors            = m_MainMenuTab_OverrideMenuEnvColors->Element()->GetValue();
        l_MainMenuConfig->BaseColor                        = m_MainMenuTab_BaseColor->Element()->GetValue();
        l_MainMenuConfig->LevelClearedColor                = m_MainMenuTab_LevelClearedColor->Element()->GetValue();
        l_MainMenuConfig->LevelFailedColor                 = m_MainMenuTab_LevelFailedColor->Element()->GetValue();
        l_MainMenuConfig->RemoveNewContentPromotional      = m_MainMenuTab_RemoveNewContentPromotional->Element()->GetValue();
        l_MainMenuConfig->DisableFireworks                 = m_MainMenuTab_DisableFireworks->Element()->GetValue();

        m_MainMenuTab_BaseColor         ->SetInteractable(l_MainMenuConfig->OverrideMenuEnvColors);
        m_MainMenuTab_LevelClearedColor ->SetInteractable(l_MainMenuConfig->OverrideMenuEnvColors);
        m_MainMenuTab_LevelFailedColor  ->SetInteractable(l_MainMenuConfig->OverrideMenuEnvColors);
        #pragma endregion MainMenu_Tab

        /// Update patches
        GameTweaker::Instance()->UpdatePatches(false);
    }
    /// @brief When settings are changed
    void SettingsMainView::OnSettingChangedOverrideMenuColor()
    {
        OnSettingChanged();
        Managers::CustomMenuLightManager::UpdateFromConfig();
        Managers::CustomMenuLightManager::SwitchToBase();
    }
    /// @brief When settings are changed
    void SettingsMainView::OnSettingChangedLevelClearedColor()
    {
        OnSettingChanged();
        Managers::CustomMenuLightManager::UpdateFromConfig();
        Managers::CustomMenuLightManager::SwitchToLevelCleared();
    }
    /// @brief When settings are changed
    void SettingsMainView::OnSettingChangedLevelFailedColor()
    {
        OnSettingChanged();
        Managers::CustomMenuLightManager::UpdateFromConfig();
        Managers::CustomMenuLightManager::SwitchToLevelFailed();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// Reset settings
    void SettingsMainView::RefreshSettings()
    {
        m_PreventChanges = true;

        auto l_Config = GTConfig::Instance();
        #pragma region Particles_tab
        m_ParticlesTab_RemoveDebris                 ->SetValue(l_Config->RemoveDebris);
        m_ParticlesTab_RemoveCutParticles           ->SetValue(l_Config->RemoveAllCutParticles);
        m_ParticlesTab_RemoveObstacleParticles      ->SetValue(l_Config->RemoveObstacleParticles);
        m_ParticlesTab_RemoveFloorBurnMarkParticles ->SetValue(l_Config->RemoveSaberBurnMarkSparkles);
        m_ParticlesTab_RemoveFloorBurnMarkEffects   ->SetValue(l_Config->RemoveSaberBurnMarks);
        m_ParticlesTab_RemoveSaberClashEffects      ->SetValue(l_Config->RemoveSaberClashEffects);
        m_ParticlesTab_RemoveWorldParticles         ->SetValue(l_Config->RemoveWorldParticles);
        #pragma endregion Particles_tab

        #pragma region Environment_Tab
        m_EnvironmentTab_RemoveMusicBandLogo            ->SetValue(l_Config->Environment->RemoveMusicBandLogo);
        m_EnvironmentTab_RemoveFullComboLossAnimation   ->SetValue(l_Config->Environment->RemoveFullComboLossAnimation);
        #pragma endregion Environment_Tab

        #pragma region LevelSelection_Tab
        m_LevelSelectionTab_RemoveBaseGameFilterButton  ->SetValue(l_Config->LevelSelection->RemoveBaseGameFilterButton);
        m_LevelSelectionTab_HighlightPlayedSong         ->SetValue(l_Config->LevelSelection->HighlightEnabled);
        m_LevelSelectionTab_HighlightPlayedSongColor    ->SetValue(ColorU::Convert(l_Config->LevelSelection->HighlightPlayed));
        m_LevelSelectionTab_HighlightPlayedSongAllColor ->SetValue(ColorU::Convert(l_Config->LevelSelection->HighlightAllPlayed));
        #pragma endregion LevelSelection_Tab

        #pragma region PlayerOptions_Tab
        m_PlayerOptionsTab_ReorderPlayerSettings            ->SetValue(l_Config->PlayerOptions->ReorderPlayerSettings);
        m_PlayerOptionsTab_AddOverrideLightIntensityOption  ->SetValue(l_Config->PlayerOptions->OverrideLightIntensityOption);
        m_PlayerOptionsTab_MergeLightEffectFilterOptions    ->SetValue(l_Config->PlayerOptions->MergeLightPressetOptions);
        #pragma endregion PlayerOptions_Tab

        #pragma region MainMenu_Tab
        m_MainMenuTab_OverrideMenuEnvColors         ->SetValue(l_Config->MainMenu->OverrideMenuEnvColors);
        m_MainMenuTab_BaseColor                     ->SetValue(l_Config->MainMenu->BaseColor);
        m_MainMenuTab_LevelClearedColor             ->SetValue(l_Config->MainMenu->LevelClearedColor);
        m_MainMenuTab_LevelFailedColor              ->SetValue(l_Config->MainMenu->LevelFailedColor);
        m_MainMenuTab_RemoveNewContentPromotional   ->SetValue(l_Config->MainMenu->RemoveNewContentPromotional);
        m_MainMenuTab_DisableFireworks              ->SetValue(l_Config->MainMenu->DisableFireworks);
        #pragma endregion MainMenu_Tab

        m_PreventChanges = false;

        /// Diagnostic: do not call OnSettingChanged() immediately after refresh.
        /// This prevents a second patch application during UI reactivation.
        Logger::Instance->Info(u"[GameTweaker-UI-Diag] Skipped OnSettingChanged after RefreshSettings");
    }

}   ///< namespace QBeatSaberPlus_GameTweaker::UI
