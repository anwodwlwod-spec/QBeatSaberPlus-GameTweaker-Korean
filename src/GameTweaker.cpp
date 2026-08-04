#include "GameTweaker.hpp"
#include "Components/MusicBandLogoRemover.hpp"
#include "Managers/CustomMenuLightManager.hpp"
#include "Patches/Lights/PLightsPatches.hpp"
#include "Patches/PLevelSearchViewController.hpp"
#include "Patches/PMainMenuViewController.hpp"
#include "Patches/PMusicPackPromoBanner.hpp"
#include "Patches/PNoteCutCoreEffectsSpawner.hpp"
#include "Patches/PNoteDebrisSpawner.hpp"
#include "Patches/PObstacleSaberSparkleEffectManager.hpp"
#include "Patches/PPlayerSettingsPanelController.hpp"
#include "Patches/PSaberBurnMarkArea.hpp"
#include "Patches/PSaberBurnMarkSparkles.hpp"
#include "Patches/PSaberClashEffect.hpp"
#include "Logger.hpp"

#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/GameObject.hpp>

using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker {

    CP_SDK_MODULE_BASE_INSTANCE_IMPL(GameTweaker);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// Constructor
    GameTweaker::GameTweaker()
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Enable the Module
    void GameTweaker::OnEnable()
    {
        Logger::Instance->Info(u"[GT-FULL-DIAG] OnEnable START");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Bind OnSceneChange BEFORE");
        CP_SDK_BS::Game::Logic::OnSceneChange += {this, &GameTweaker::Game_OnSceneChange};
        Logger::Instance->Info(u"[GT-FULL-DIAG] Bind OnSceneChange AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Bind OnLevelStarted BEFORE");
        CP_SDK_BS::Game::Logic::OnLevelStarted += {this, &GameTweaker::Game_OnLevelStarted};
        Logger::Instance->Info(u"[GT-FULL-DIAG] Bind OnLevelStarted AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] CustomMenuLightManager::Init BEFORE");
        Managers::CustomMenuLightManager::Init();
        Logger::Instance->Info(u"[GT-FULL-DIAG] CustomMenuLightManager::Init AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] UpdatePatches(false) BEFORE");
        UpdatePatches(false);
        Logger::Instance->Info(u"[GT-FULL-DIAG] UpdatePatches(false) AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] OnEnable END");
    }
    /// @brief Disable the Module
    void GameTweaker::OnDisable()
    {
        Logger::Instance->Info(u"[GT-FULL-DIAG] OnDisable START");

        Logger::Instance->Info(u"[GT-FULL-DIAG] UpdatePatches(true) BEFORE");
        UpdatePatches(true);
        Logger::Instance->Info(u"[GT-FULL-DIAG] UpdatePatches(true) AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Destroy SettingsLeftView BEFORE");
        CP_SDK::UI::UISystem::DestroyUI(&m_SettingsLeftView);
        Logger::Instance->Info(u"[GT-FULL-DIAG] Destroy SettingsLeftView AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Destroy SettingsMainView BEFORE");
        CP_SDK::UI::UISystem::DestroyUI(&m_SettingsMainView);
        Logger::Instance->Info(u"[GT-FULL-DIAG] Destroy SettingsMainView AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Unbind OnSceneChange BEFORE");
        CP_SDK_BS::Game::Logic::OnSceneChange -= {this, &GameTweaker::Game_OnSceneChange};
        Logger::Instance->Info(u"[GT-FULL-DIAG] Unbind OnSceneChange AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] Unbind OnLevelStarted BEFORE");
        CP_SDK_BS::Game::Logic::OnLevelStarted -= {this, &GameTweaker::Game_OnLevelStarted};
        Logger::Instance->Info(u"[GT-FULL-DIAG] Unbind OnLevelStarted AFTER");

        Logger::Instance->Info(u"[GT-FULL-DIAG] OnDisable END");
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Get Module settings UI
    GameTweaker::t_InitialViews GameTweaker::GetSettingsViewControllers()
    {
        Logger::Instance->Info(u"[GT-FULL-DIAG] GetSettingsViewControllers START");

        if (!m_SettingsMainView)
        {
            Logger::Instance->Info(u"[GT-FULL-DIAG] Create SettingsMainView BEFORE");
            m_SettingsMainView = CP_SDK::UI::UISystem::CreateViewController<UI::SettingsMainView*>();
            Logger::Instance->Info(u"[GT-FULL-DIAG] Create SettingsMainView AFTER");
        }
        else
        {
            Logger::Instance->Info(u"[GT-FULL-DIAG] Reuse SettingsMainView");
        }

        if (!m_SettingsLeftView)
        {
            Logger::Instance->Info(u"[GT-FULL-DIAG] Create SettingsLeftView BEFORE");
            m_SettingsLeftView = CP_SDK::UI::UISystem::CreateViewController<UI::SettingsLeftView*>();
            Logger::Instance->Info(u"[GT-FULL-DIAG] Create SettingsLeftView AFTER");
        }
        else
        {
            Logger::Instance->Info(u"[GT-FULL-DIAG] Reuse SettingsLeftView");
        }

        Logger::Instance->Info(u"[GT-FULL-DIAG] GetSettingsViewControllers END");
        return { m_SettingsMainView.Ptr(), m_SettingsLeftView.Ptr(), nullptr};
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Update patches
    /// @param p_ForceDisable Force disable
    void GameTweaker::UpdatePatches(bool p_ForceDisable)
    {
        Logger::Instance->Info(
            p_ForceDisable
                ? u"[GT-FULL-DIAG] UpdatePatches START DISABLE"
                : u"[GT-FULL-DIAG] UpdatePatches START ENABLE"
        );

        #define GT_DIAG_PATCH(NAME, CALL) \
            try { \
                Logger::Instance->Info(u"[GT-FULL-DIAG] " NAME u" BEFORE"); \
                CALL; \
                Logger::Instance->Info(u"[GT-FULL-DIAG] " NAME u" AFTER"); \
            } catch (const std::exception& p_PatchException) { \
                Logger::Instance->Error(u"[GT-FULL-DIAG] " NAME u" EXCEPTION"); \
                Logger::Instance->Error(p_PatchException); \
            }

        GT_DIAG_PATCH(
            u"PNoteCutCoreEffectsSpawner",
            Patches::PNoteCutCoreEffectsSpawner::SetRemoveCutParticles(
                !p_ForceDisable && GTConfig::Instance()->RemoveAllCutParticles
            )
        );

        GT_DIAG_PATCH(
            u"PObstacleSaberSparkleEffectManager",
            Patches::PObstacleSaberSparkleEffectManager::SetRemoveObstacleParticles(
                !p_ForceDisable && GTConfig::Instance()->RemoveObstacleParticles
            )
        );

        GT_DIAG_PATCH(
            u"PSaberBurnMarkSparkles",
            Patches::PSaberBurnMarkSparkles::SetRemoveSaberBurnMarkSparkles(
                !p_ForceDisable && GTConfig::Instance()->RemoveSaberBurnMarkSparkles
            )
        );

        GT_DIAG_PATCH(
            u"PSaberBurnMarkArea",
            Patches::PSaberBurnMarkArea::SetRemoveSaberBurnMarks(
                !p_ForceDisable && GTConfig::Instance()->RemoveSaberBurnMarks
            )
        );

        GT_DIAG_PATCH(
            u"PSaberClashEffect",
            Patches::PSaberClashEffect::SetRemoveClashEffects(
                !p_ForceDisable && GTConfig::Instance()->RemoveSaberClashEffects
            )
        );

        GT_DIAG_PATCH(
            u"UpdateWorldParticles",
            UpdateWorldParticles(p_ForceDisable)
        );

        GT_DIAG_PATCH(
            u"PMainMenuViewController",
            Patches::PMainMenuViewController::SetRemovePackMusicPromoBanner(
                !p_ForceDisable && GTConfig::Instance()->MainMenu->RemoveNewContentPromotional
            )
        );

        GT_DIAG_PATCH(
            u"PMusicPackPromoBanner",
            Patches::PMusicPackPromoBanner::SetEnabled(
                !p_ForceDisable && GTConfig::Instance()->MainMenu->RemoveNewContentPromotional
            )
        );

        GT_DIAG_PATCH(
            u"PPlayerSettingsPanelController SetReorderEnabled",
            Patches::PPlayerSettingsPanelController::SetReorderEnabled(
                !p_ForceDisable && GTConfig::Instance()->PlayerOptions->ReorderPlayerSettings,
                !p_ForceDisable && GTConfig::Instance()->PlayerOptions->OverrideLightIntensityOption
            )
        );

        GT_DIAG_PATCH(
            u"PPlayerSettingsPanelController SetLightsOptionMerging",
            Patches::PPlayerSettingsPanelController::SetLightsOptionMerging(
                !p_ForceDisable && GTConfig::Instance()->PlayerOptions->MergeLightPressetOptions
            )
        );

        GT_DIAG_PATCH(
            u"PLevelSearchViewController",
            Patches::PLevelSearchViewController::SetRemoveBaseGameFilter(
                !p_ForceDisable && GTConfig::Instance()->LevelSelection->RemoveBaseGameFilterButton
            )
        );

        #undef GT_DIAG_PATCH

        Logger::Instance->Info(u"[GT-FULL-DIAG] UpdatePatches END");
    }
    /// @brief Update world particles
    /// @param p_ForceDisable Force disable
    void GameTweaker::UpdateWorldParticles(bool p_ForceDisable)
    {
        auto l_Objects = Resources::FindObjectsOfTypeAll<GameObject*>();
        for (auto& l_Current : l_Objects)
        {
            if (l_Current->get_name() != "DustPS")
                continue;

            l_Current->SetActive(p_ForceDisable ? true : !GTConfig::Instance()->RemoveWorldParticles);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On game scene change
    /// @param p_Scene New scene
    void GameTweaker::Game_OnSceneChange(CP_SDK_BS::Game::Logic::ESceneType p_Scene)
    {
        Patches::Lights::PLightsPatches::SetIsValidScene(p_Scene == CP_SDK_BS::Game::Logic::ESceneType::Playing);
        UpdateWorldParticles();
    }
    /// @brief On level started
    /// @param p_LevelData Level data
    void GameTweaker::Game_OnLevelStarted(const CP_SDK_BS::Game::LevelData::Ptr& p_LevelData)
    {
        Patches::Lights::PLightsPatches::SetFromConfig();
        Patches::PNoteDebrisSpawner::SetFromConfig();

        if (GTConfig::Instance()->Environment->RemoveMusicBandLogo && p_LevelData && p_LevelData->Data && p_LevelData->Data->targetEnvironmentInfo)
        {
            if (   p_LevelData->Data->targetEnvironmentInfo->____serializedName == "BTSEnvironment"
                || p_LevelData->Data->targetEnvironmentInfo->____serializedName == "LinkinParkEnvironment")
            {
                GameObject::New_ctor("BeatSaberPlus_MusicBandLogoRemover")->AddComponent<Components::MusicBandLogoRemover*>();
            }
        }
    }

}   ///< namespace QBeatSaberPlus_GameTweaker
