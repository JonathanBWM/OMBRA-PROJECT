#include "spoofer_ui.h"
#include "imgui.h"

namespace Ombra {

SpooferUI::SpooferUI()
    : m_initialized(false)
    , m_applied(false)
{}

SpooferUI::~SpooferUI() {
    Shutdown();
}

bool SpooferUI::Initialize() {
    // STUB: No actual initialization needed yet
    m_initialized = true;
    return true;
}

void SpooferUI::Shutdown() {
    // STUB: No cleanup needed yet
    m_initialized = false;
    m_applied = false;
}

void SpooferUI::Render() {
    // STUB: Placeholder UI
    ImGui::Text("Spoofer functionality coming soon...");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("Hardware spoofing features will be implemented in a future update.");
    ImGui::TextDisabled("This includes disk serial, MAC address, and registry spoofing.");
}

} // namespace Ombra
