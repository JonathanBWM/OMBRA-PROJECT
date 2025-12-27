#pragma once

namespace Ombra {

class SpooferUI {
public:
    SpooferUI();
    ~SpooferUI();

    bool Initialize();
    void Shutdown();
    void Render();

    bool IsInitialized() const { return m_initialized; }
    bool IsApplied() const { return m_applied; }

private:
    bool m_initialized;
    bool m_applied;
};

} // namespace Ombra
