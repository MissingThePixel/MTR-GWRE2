// gw2_recompiled - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <rex/rex_app.h>
#include <rex/runtime.h>
#include <rex/system/xam/content_manager.h>
#include <rex/system/xam/user_profile.h>
#include <cstring>

class Gw2RecompiledApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<Gw2RecompiledApp>(new Gw2RecompiledApp(ctx, "gw2_recompiled",
        PPCImageConfig));
  }

  // Override virtual hooks for customization:
  // void OnPostInitLogging() override {}
  // void OnPreSetup(rex::RuntimeConfig& config) override {}
  // void OnLoadXexImage(std::string& xex_image) override {}
  // void OnPostLoadXexImage() override {}
  void OnPostSetup() override {
    auto* kernel = runtime()->kernel_state();
    auto* profile = kernel->user_profile();
    const auto path = kernel->content_manager()->ResolveGameUserContentPath() / "63E83FFF";
    if (std::filesystem::exists(path)) return;

    // Exact 48-byte defaults from guest sub_82093230: control=2,
    // music/effects volume from guest constant 820006C4, mode flags=2,
    // no scores or unlocks. Seed only missing profiles; never replace saves.
    std::vector<uint8_t> defaults(48, 0);
    defaults[3] = 2;
    defaults[19] = 2;
    const auto* guest = runtime()->virtual_membase();
    std::memcpy(defaults.data() + 4, guest + 0x820006C4, 4);
    std::memcpy(defaults.data() + 8, guest + 0x820006C4, 4);
    profile->AddSetting(std::make_unique<rex::system::xam::UserProfile::BinarySetting>(
        0x63E83FFF, defaults));
  }
  // void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {}
  // std::unique_ptr<rex::ui::ImGuiDialog> CreateAchievementsOverlay() override;
  std::unique_ptr<rex::ui::AchievementNotificationDialog>
  CreateAchievementNotificationDialog() override {
    // Match PGR4: avoid a permanent UI drawer that changes presentation pacing.
    return nullptr;
  }
  // void OnShutdown() override {}
  // void OnConfigurePaths(rex::PathConfig& paths) override {}
};
