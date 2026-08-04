#include "UI/SettingsLeftView.hpp"
#include "UI/SettingsMainView.hpp"
#include "GameTweaker.hpp"

using namespace CP_SDK::XUI;
using namespace UnityEngine;

namespace QBeatSaberPlus_GameTweaker::UI {

    constexpr const std::u16string_view s_InformationStr =
               u"<line-height=125%><b>게임 트위커</b>"
         u"\n" u"<i><color=#CCCCCCFF>이 모듈은 게임 환경을 원하는 대로 조정하고, 불필요한 기본 기능을 제거하며, 새로운 편의 기능과 세부 조정 기능을 추가합니다.</color></i>";

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    CP_SDK_IL2CPP_INHERIT_INIT(SettingsLeftView);

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Constructor
    CP_SDK_IL2CPP_DECLARE_CTOR_IMPL(SettingsLeftView)
    {
        OnViewCreation = {this, &SettingsLeftView::OnViewCreation_Impl};
    }
    /// @brief Destructor
    CP_SDK_IL2CPP_DECLARE_DTOR_MONOBEHAVIOUR_IMPL(SettingsLeftView)
    {

    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief On view creation
    void SettingsLeftView::OnViewCreation_Impl()
    {
        Templates::FullRectLayout({
            Templates::TitleBar(u"정보 / 제작진"),

            Templates::ScrollableInfos(50.0f, {
                XUIText::Make(s_InformationStr)
                    ->SetAlign(TMPro::TextAlignmentOptions::Left)
                    ->AsShared()
            }),

            Templates::ExpandedButtonsLine({
                XUIPrimaryButton::Make(u"초기화", {this, &SettingsLeftView::OnResetButton})->AsShared(),
            }),
            Templates::ExpandedButtonsLine({
                XUISecondaryButton::Make(u"사용 설명서", {this, &SettingsLeftView::OnDocumentationButton})->AsShared()
            })
        })
        ->SetBackground(true, std::nullopt, true)
        ->BuildUI(get_transform());
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Reset button
    void SettingsLeftView::OnResetButton()
    {
        ShowConfirmationModal(
            u"<color=yellow><b>게임 트위커 설정을 정말\n초기화하시겠습니까?",
            [this](bool p_Confirm) -> void
            {
                if (!p_Confirm)
                    return;

                GTConfig::Instance()->Reset();
                GTConfig::Instance()->Enabled = true;
                GTConfig::Instance()->Save();

                /// Refresh values
                SettingsMainView::Instance()->RefreshSettings();
            }
        );
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// @brief Documentation button
    void SettingsLeftView::OnDocumentationButton()
    {
        ShowMessageModal(u"웹 브라우저에서 주소를 열었습니다.");
        CP_SDK::ChatPlexSDK::OpenURL(GameTweaker::Instance()->DocumentationURL());
    }

}   ///< namespace QBeatSaberPlus_GameTweaker::UI