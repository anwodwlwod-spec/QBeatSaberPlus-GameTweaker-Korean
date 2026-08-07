#pragma once

#include_next <TMPro/TMP_FontAsset.hpp>

// Compatibility shim for the bs-cordl enum wrapper used by this project.
// SettingsMainView.cpp refers to AtlasPopulationMode_Unwrapped, while the
// generated header exposes __AtlasPopulationMode_Unwrapped.
#define AtlasPopulationMode_Unwrapped __AtlasPopulationMode_Unwrapped
