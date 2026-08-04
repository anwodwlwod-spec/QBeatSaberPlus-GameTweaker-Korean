#include "Managers/CustomMenuLightManager.hpp"
#include "GTConfig.hpp"
#include "Logger.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>

#include <algorithm>

#include <GlobalNamespace/MenuLightsPresetSO.hpp>
#include <GlobalNamespace/SoloFreePlayFlowCoordinator.hpp>
#include <UnityEngine/GameObject.hpp>

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker::Managers {

    CP_SDK_IL2CPP_INHERIT_INIT(CustomColorSO);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    Color CustomColorSO::ColorSO__get_color()
    {
        return _color;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    _u::MonoPtr<MenuLightsManager>                          CustomMenuLightManager::m_MenuLightsManager;
    _u::MonoPtr<MenuLightsPresetSO>                         CustomMenuLightManager::m_DefaultPreset;
    _u::MonoPtr<MenuLightsPresetSO>                         CustomMenuLightManager::m_LevelClearedPreset;
    _u::MonoPtr<MenuLightsPresetSO>                         CustomMenuLightManager::m_LevelFailedPreset;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>>    CustomMenuLightManager::m_DefaultPresetBackup;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>>    CustomMenuLightManager::m_LevelClearedPresetBackup;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>>    CustomMenuLightManager::m_LevelFailedPresetBackup;
    _u::MonoPtr<CustomColorSO>                              CustomMenuLightManager::m_Custom_Default;
    _u::MonoPtr<CustomColorSO>                              CustomMenuLightManager::m_Custom_Cleared;
    _u::MonoPtr<CustomColorSO>                              CustomMenuLightManager::m_Custom_Failed;

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Init and start reflection fetching
    void CustomMenuLightManager::Init()
    {
        Logger::Instance->Info(u"[GT-LIGHT-FIX] Init START");

        // This manager keeps static state for the lifetime of the process.
        // Re-enabling the module must not create another set of ColorSO objects
        // or start a second initialization coroutine.
        if (m_Custom_Default)
        {
            Logger::Instance->Info(u"[GT-LIGHT-FIX] Already initialized - reuse existing state");
            UpdateFromConfig();
            Logger::Instance->Info(u"[GT-LIGHT-FIX] Reuse UpdateFromConfig AFTER");
            return;
        }

        // Ensure the first initialization always starts with empty backups.
        m_DefaultPresetBackup.clear();
        m_LevelClearedPresetBackup.clear();
        m_LevelFailedPresetBackup.clear();

        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Default BEFORE");
        m_Custom_Default = CustomColorSO::CreateInstance<CustomColorSO*>();
        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Default AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Cleared BEFORE");
        m_Custom_Cleared = CustomColorSO::CreateInstance<CustomColorSO*>();
        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Cleared AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Failed BEFORE");
        m_Custom_Failed = CustomColorSO::CreateInstance<CustomColorSO*>();
        Logger::Instance->Info(u"[GT-LIGHT-FIX] Create Custom_Failed AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-FIX] Start Coroutine_InitLate BEFORE");
        CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
            Coroutine_InitLate()
        ));
        Logger::Instance->Info(u"[GT-LIGHT-FIX] Start Coroutine_InitLate AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-FIX] Init END");
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Apply current config
    void CustomMenuLightManager::UpdateFromConfig()
    {
        if (!m_DefaultPreset)
            return;

        auto l_Enabled = GTConfig::Instance()->MainMenu->OverrideMenuEnvColors;
        if (!l_Enabled)
        {
            const int l_DefaultCount = std::min(
                static_cast<int>(m_DefaultPresetBackup.size()),
                static_cast<int>(m_DefaultPreset->____lightIdColorPairs->get_Length())
            );

            for (int l_I = 0; l_I < l_DefaultCount; ++l_I)
            {
                m_DefaultPreset->____lightIdColorPairs[l_I]->___intensity = std::get<0>(m_DefaultPresetBackup[l_I]);
                m_DefaultPreset->____lightIdColorPairs[l_I]->___baseColor = std::get<1>(m_DefaultPresetBackup[l_I]).Ptr();
            }

            if (m_LevelClearedPreset && m_LevelFailedPreset)
            {
                const int l_ClearedCount = std::min(
                    static_cast<int>(m_LevelClearedPresetBackup.size()),
                    static_cast<int>(m_LevelClearedPreset->____lightIdColorPairs->get_Length())
                );

                for (int l_I = 0; l_I < l_ClearedCount; ++l_I)
                {
                    m_LevelClearedPreset->____lightIdColorPairs[l_I]->___intensity = std::get<0>(m_LevelClearedPresetBackup[l_I]);
                    m_LevelClearedPreset->____lightIdColorPairs[l_I]->___baseColor = std::get<1>(m_LevelClearedPresetBackup[l_I]).Ptr();
                }

                const int l_FailedCount = std::min(
                    static_cast<int>(m_LevelFailedPresetBackup.size()),
                    static_cast<int>(m_LevelFailedPreset->____lightIdColorPairs->get_Length())
                );

                for (int l_I = 0; l_I < l_FailedCount; ++l_I)
                {
                    m_LevelFailedPreset->____lightIdColorPairs[l_I]->___intensity = std::get<0>(m_LevelFailedPresetBackup[l_I]);
                    m_LevelFailedPreset->____lightIdColorPairs[l_I]->___baseColor = std::get<1>(m_LevelFailedPresetBackup[l_I]).Ptr();
                }
            }
        }
        else
        {
            m_Custom_Default->_color = GTConfig::Instance()->MainMenu->BaseColor;
            m_Custom_Cleared->_color = GTConfig::Instance()->MainMenu->LevelClearedColor;
            m_Custom_Failed->_color  = GTConfig::Instance()->MainMenu->LevelFailedColor;

            for (int l_I = 0; l_I < m_DefaultPreset->____lightIdColorPairs->get_Length(); ++l_I)
                m_DefaultPreset->____lightIdColorPairs[l_I]->___baseColor = m_Custom_Default.Ptr();

            if (m_LevelClearedPreset && m_LevelFailedPreset)
            {
                for (int l_I = 0; l_I < m_LevelClearedPreset->____lightIdColorPairs->get_Length(); ++l_I)
                    m_LevelClearedPreset->____lightIdColorPairs[l_I]->___baseColor = m_Custom_Cleared.Ptr();

                for (int l_I = 0; l_I < m_LevelFailedPreset->____lightIdColorPairs->get_Length(); ++l_I)
                    m_LevelFailedPreset->____lightIdColorPairs[l_I]->___baseColor = m_Custom_Failed.Ptr();
            }
        }

        try
        {
            if (m_MenuLightsManager && m_DefaultPreset)
            {
                m_MenuLightsManager->____preset = m_DefaultPreset.Ptr();
                m_MenuLightsManager->set_enabled(true);
                m_MenuLightsManager->RefreshColors();
            }
        }
        catch (const std::exception&)
        {

        }
    }
    /// @brief Switch to base preset
    void CustomMenuLightManager::SwitchToBase()
    {
        if (!GTConfig::Instance()->MainMenu->OverrideMenuEnvColors)
            return;

        try
        {
            if (m_MenuLightsManager && m_DefaultPreset)
            {
                m_MenuLightsManager->____preset = m_DefaultPreset.Ptr();
                m_MenuLightsManager->set_enabled(true);
                m_MenuLightsManager->RefreshColors();
            }
        }
        catch (const std::exception&)
        {

        }
    }
    /// @brief Switch to level cleared preset
    void CustomMenuLightManager::SwitchToLevelCleared()
    {
        if (!GTConfig::Instance()->MainMenu->OverrideMenuEnvColors)
            return;

        try
        {
            if (m_MenuLightsManager && m_LevelClearedPreset)
            {
                m_MenuLightsManager->____preset = m_LevelClearedPreset.Ptr();
                m_MenuLightsManager->set_enabled(true);
                m_MenuLightsManager->RefreshColors();
            }
        }
        catch (const std::exception&)
        {

        }
    }
    /// @brief Switch to level failed preset
    void CustomMenuLightManager::SwitchToLevelFailed()
    {
        if (!GTConfig::Instance()->MainMenu->OverrideMenuEnvColors)
            return;

        try
        {
            if (m_MenuLightsManager && m_LevelFailedPreset)
            {
                m_MenuLightsManager->____preset = m_LevelFailedPreset.Ptr();
                m_MenuLightsManager->set_enabled(true);
                m_MenuLightsManager->RefreshColors();
            }
        }
        catch (const std::exception&)
        {

        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Init coroutine that fetch object by reflection
    custom_types::Helpers::Coroutine CustomMenuLightManager::Coroutine_InitLate()
    {
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Coroutine_InitLate START");

        while (!m_MenuLightsManager)
        {
            co_yield nullptr;

            auto l_Found = GameObject::FindObjectOfType<MenuLightsManager*>();
            if (l_Found)
            {
                Logger::Instance->Info(u"[GT-LIGHT-DIAG] MenuLightsManager FOUND");
                m_MenuLightsManager = l_Found;
            }
        }

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Read default preset BEFORE");
        m_DefaultPreset = m_MenuLightsManager->____defaultPreset;
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Read default preset AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup default preset BEFORE");
        for (auto& l_Current : m_DefaultPreset->____lightIdColorPairs)
            m_DefaultPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup default preset AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] First UpdateFromConfig BEFORE");
        UpdateFromConfig();
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] First UpdateFromConfig AFTER");

        SoloFreePlayFlowCoordinator* l_SoloFreePlayFlowCoordinator = nullptr;
        while (!l_SoloFreePlayFlowCoordinator)
        {
            co_yield nullptr;
            l_SoloFreePlayFlowCoordinator = GameObject::FindObjectOfType<SoloFreePlayFlowCoordinator*>();
            if (l_SoloFreePlayFlowCoordinator)
                Logger::Instance->Info(u"[GT-LIGHT-DIAG] SoloFreePlayFlowCoordinator FOUND");
        }

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Read result presets BEFORE");
        m_LevelClearedPreset = l_SoloFreePlayFlowCoordinator->____resultsClearedLightsPreset;
        m_LevelFailedPreset  = l_SoloFreePlayFlowCoordinator->____resultsFailedLightsPreset;
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Read result presets AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup cleared preset BEFORE");
        for (auto& l_Current : m_LevelClearedPreset->____lightIdColorPairs)
            m_LevelClearedPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup cleared preset AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup failed preset BEFORE");
        for (auto& l_Current : m_LevelFailedPreset->____lightIdColorPairs)
            m_LevelFailedPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Backup failed preset AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Second UpdateFromConfig BEFORE");
        UpdateFromConfig();
        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Second UpdateFromConfig AFTER");

        Logger::Instance->Info(u"[GT-LIGHT-DIAG] Coroutine_InitLate END");
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Preset replacer for PMenuLightsManager harmony patch
    /// @param preset Input preset
    MenuLightsPresetSO* CustomMenuLightManager::GetPresetForPatch(MenuLightsPresetSO* preset)
    {
        if (!m_Custom_Default || !GTConfig::Instance()->MainMenu->OverrideMenuEnvColors)
            return preset;

        if (m_LevelClearedPreset && m_LevelFailedPreset && (preset->get_name() == m_LevelClearedPreset->get_name() || preset->get_name() == m_LevelFailedPreset->get_name()))
            return preset;

        return m_DefaultPreset.Ptr();
    }

}   ///< namespace QBeatSaberPlus_GameTweaker::Managers
