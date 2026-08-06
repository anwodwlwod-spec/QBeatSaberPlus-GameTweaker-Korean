#include "UI/SettingsMainView.hpp"
#include "Managers/CustomMenuLightManager.hpp"
#include "GameTweaker.hpp"
#include "GTConfig.hpp"
#include "Logger.hpp"
#include "assets.hpp"

#include <CP_SDK/UI/ValueFormatters.hpp>
#include <CP_SDK/Unity/Extensions/ColorU.hpp>

#include <TMPro/TMP_FontAsset.hpp>
#include <TMPro/TMP_Text.hpp>
#include <UnityEngine/AssetBundle.hpp>
#include <UnityEngine/Object.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Transform.hpp>
#include <beatsaber-hook/shared/utils/il2cpp-functions.hpp>

using namespace CP_SDK::Unity::Extensions;
using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker::UI {

    static UnityEngine::AssetBundle* s_KoreanFontBundle = nullptr;
    static TMPro::TMP_FontAsset* s_KoreanFont = nullptr;

    /// @brief Return true when the font directly contains every Korean glyph that
    ///        was previously rendered at a smaller fallback size.
    static bool SupportsProblematicKoreanGlyphs(TMPro::TMP_FontAsset* p_Font)
    {
        return p_Font
            && p_Font->HasCharacter(u'룹', false, false)
            && p_Font->HasCharacter(u'콤', false, false)
            && p_Font->HasCharacter(u'밝', false, false)
            && p_Font->HasCharacter(u'홍', false, false)
            && p_Font->HasCharacter(u'죽', false, false);
    }

    /// @brief Load the CP_SDK font AssetBundle embedded in this mod.
    static TMPro::TMP_FontAsset* LoadKoreanFont()
    {
        if (s_KoreanFont)
            return s_KoreanFont;

        try
        {
            using t_LoadFromMemory = UnityEngine::AssetBundle* (*)(::Array<uint8_t>*, unsigned int);

            static auto s_LoadFromMemory = reinterpret_cast<t_LoadFromMemory>(
                il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemory_Internal")
            );

            if (!s_LoadFromMemory)
            {
                Logger::Instance->Error(u"[KoreanFont] AssetBundle LoadFromMemory icall was not found.");
                return nullptr;
            }

            s_KoreanFontBundle = s_LoadFromMemory(IncludedAssets::QuestFonts_bundle.Raw(), 0);
            if (!s_KoreanFontBundle)
            {
                Logger::Instance->Error(u"[KoreanFont] Failed to load QuestFonts.bundle.");
                return nullptr;
            }

            UnityEngine::Object::DontDestroyOnLoad(s_KoreanFontBundle);

            auto* l_MainFont = s_KoreanFontBundle->LoadAsset<TMPro::TMP_FontAsset*>(
                u"[CP_SDK]segoeui SDF Main"
            );
            auto* l_ChatFont = s_KoreanFontBundle->LoadAsset<TMPro::TMP_FontAsset*>(
                u"[CP_SDK]segoeui SDF"
            );

            if (SupportsProblematicKoreanGlyphs(l_MainFont))
                s_KoreanFont = l_MainFont;
            else if (SupportsProblematicKoreanGlyphs(l_ChatFont))
                s_KoreanFont = l_ChatFont;
            else
            {
                Logger::Instance->Error(
                    u"[KoreanFont] QuestFonts.bundle loaded, but neither bundled font contains all required Korean glyphs."
                );
                return nullptr;
            }

            UnityEngine::Object::DontDestroyOnLoad(s_KoreanFont);
            std::u16string l_Message = u"[KoreanFont] Loaded font: ";
            l_Message += s_KoreanFont->get_name();
            Logger::Instance->Info(l_Message);

            return s_KoreanFont;
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanFont] Font loading failed:");
            Logger::Instance->Error(l_Exception);
            return nullptr;
        }
    }

    /// @brief Apply the bundled font only to TMP texts belonging to this view.
    static void ApplyKoreanFontToView(UnityEngine::Transform* p_ViewTransform)
    {
        auto* l_Font = LoadKoreanFont();
        if (!l_Font || !p_ViewTransform)
            return;

        try
        {
            auto l_Texts = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_Text*>();
            int l_AppliedCount = 0;

            for (auto& l_Text : l_Texts)
            {
                if (!l_Text)
                    continue;

                const std::u16string l_Value = l_Text->get_text();
                const bool l_IsGameTweakerText =
                    l_Value == u"게임 트위커 | 설정" ||
                    l_Value == u"파티클" ||
                    l_Value == u"환경" ||
                    l_Value == u"곡 선택" ||
                    l_Value == u"플레이어 설정" ||
                    l_Value == u"메인 메뉴" ||
                    l_Value == u"모든 노트 파편 생성 제거" ||
                    l_Value == u"환경 파티클 제거" ||
                    l_Value == u"바닥 화염 파티클 제거" ||
                    l_Value == u"노트 절단 파티클 제거" ||
                    l_Value == u"바닥 베기 자국 제거" ||
                    l_Value == u"장애물 파티클 제거" ||
                    l_Value == u"세이버 충돌 효과 제거" ||
                    l_Value == u"환경의 음악 그룹 로고 제거 (BTS, Linkin Park 등)" ||
                    l_Value == u"풀 콤보 실패 애니메이션 제거" ||
                    l_Value == u"플레이한 곡 색상" ||
                    l_Value == u"플레이한 곡 강조" ||
                    l_Value == u"모든 난이도 플레이 완료 색상" ||
                    l_Value == u"기본 게임 필터 제거" ||
                    l_Value == u"플레이어 설정 메뉴 개선" ||
                    l_Value == u"조명 밝기 재정의 옵션 추가" ||
                    l_Value == u"조명 효과 필터 옵션 통합" ||
                    l_Value == u"메뉴 환경 색상 재정의" ||
                    l_Value == u"기본 색상" ||
                    l_Value == u"새 콘텐츠 홍보 제거" ||
                    l_Value == u"레벨 성공 색상" ||
                    l_Value == u"레벨 실패 색상" ||
                    l_Value == u"폭죽 비활성화";

                if (!l_IsGameTweakerText)
                    continue;

                l_Text->set_font(l_Font);
                ++l_AppliedCount;
            }

            Logger::Instance->Info(u"[KoreanFont] Applied bundled font to GameTweaker view texts.");
        }
        catch (const std::exception& l_Exception)
        {
            Logger::Instance->Error(u"[KoreanFont] Applying font to GameTweaker view failed:");
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

        /// Apply the embedded Korean-capable font only to this settings view.
        ApplyKoreanFontToView(get_transform());

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
