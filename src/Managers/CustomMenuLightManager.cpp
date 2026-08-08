#include "Managers/CustomMenuLightManager.hpp"
#include "GTConfig.hpp"

#include <CP_SDK/Unity/MTCoroutineStarter.hpp>

#include <algorithm>

#include <GlobalNamespace/MenuLightsPresetSO.hpp>
#include <GlobalNamespace/SoloFreePlayFlowCoordinator.hpp>
#include <UnityEngine/GameObject.hpp>

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker::Managers {

    CP_SDK_IL2CPP_INHERIT_INIT(CustomColorSO);

    Color CustomColorSO::ColorSO__get_color()
    {
        return _color;
    }

    _u::MonoPtr<MenuLightsManager>                       CustomMenuLightManager::m_MenuLightsManager;
    _u::MonoPtr<MenuLightsPresetSO>                      CustomMenuLightManager::m_DefaultPreset;
    _u::MonoPtr<MenuLightsPresetSO>                      CustomMenuLightManager::m_LevelClearedPreset;
    _u::MonoPtr<MenuLightsPresetSO>                      CustomMenuLightManager::m_LevelFailedPreset;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>> CustomMenuLightManager::m_DefaultPresetBackup;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>> CustomMenuLightManager::m_LevelClearedPresetBackup;
    std::vector<std::tuple<float, _u::MonoPtr<ColorSO>>> CustomMenuLightManager::m_LevelFailedPresetBackup;
    _u::MonoPtr<CustomColorSO>                           CustomMenuLightManager::m_Custom_Default;
    _u::MonoPtr<CustomColorSO>                           CustomMenuLightManager::m_Custom_Cleared;
    _u::MonoPtr<CustomColorSO>                           CustomMenuLightManager::m_Custom_Failed;

    void CustomMenuLightManager::Init()
    {
        // Static manager state survives module disable/enable. Reuse it instead
        // of creating duplicate ColorSO instances or duplicate init coroutines.
        if (m_Custom_Default)
        {
            UpdateFromConfig();
            return;
        }

        m_DefaultPresetBackup.clear();
        m_LevelClearedPresetBackup.clear();
        m_LevelFailedPresetBackup.clear();

        m_Custom_Default = CustomColorSO::CreateInstance<CustomColorSO*>();
        m_Custom_Cleared = CustomColorSO::CreateInstance<CustomColorSO*>();
        m_Custom_Failed  = CustomColorSO::CreateInstance<CustomColorSO*>();

        CP_SDK::Unity::MTCoroutineStarter::Start(custom_types::Helpers::CoroutineHelper::New(
            Coroutine_InitLate()
        ));
    }

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

    custom_types::Helpers::Coroutine CustomMenuLightManager::Coroutine_InitLate()
    {
        while (!m_MenuLightsManager)
        {
            co_yield nullptr;

            auto l_Found = GameObject::FindObjectOfType<MenuLightsManager*>();
            if (l_Found)
                m_MenuLightsManager = l_Found;
        }

        m_DefaultPreset = m_MenuLightsManager->____defaultPreset;

        for (auto& l_Current : m_DefaultPreset->____lightIdColorPairs)
            m_DefaultPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));

        UpdateFromConfig();

        SoloFreePlayFlowCoordinator* l_SoloFreePlayFlowCoordinator = nullptr;
        while (!l_SoloFreePlayFlowCoordinator)
        {
            co_yield nullptr;
            l_SoloFreePlayFlowCoordinator = GameObject::FindObjectOfType<SoloFreePlayFlowCoordinator*>();
        }

        m_LevelClearedPreset = l_SoloFreePlayFlowCoordinator->____resultsClearedLightsPreset;
        m_LevelFailedPreset  = l_SoloFreePlayFlowCoordinator->____resultsFailedLightsPreset;

        for (auto& l_Current : m_LevelClearedPreset->____lightIdColorPairs)
            m_LevelClearedPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));

        for (auto& l_Current : m_LevelFailedPreset->____lightIdColorPairs)
            m_LevelFailedPresetBackup.push_back(std::make_tuple(l_Current->___intensity, l_Current->___baseColor.unsafePtr()));

        UpdateFromConfig();
    }

    MenuLightsPresetSO* CustomMenuLightManager::GetPresetForPatch(MenuLightsPresetSO* preset)
    {
        if (!m_Custom_Default || !GTConfig::Instance()->MainMenu->OverrideMenuEnvColors)
            return preset;

        if (m_LevelClearedPreset && m_LevelFailedPreset
            && (preset->get_name() == m_LevelClearedPreset->get_name()
                || preset->get_name() == m_LevelFailedPreset->get_name()))
            return preset;

        return m_DefaultPreset.Ptr();
    }

}   ///< namespace QBeatSaberPlus_GameTweaker::Managers
