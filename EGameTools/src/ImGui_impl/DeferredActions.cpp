#include <EGT\ImGui_impl\DeferredActions.h>

namespace EGT::ImGui_impl {
    std::vector<std::function<void()>> DeferredActions::actions{};
    std::mutex DeferredActions::mutex{};

    void DeferredActions::Add(const std::function<void()>& action) {
        std::lock_guard<std::mutex> lock(mutex);
        actions.push_back(action);
    }
    void DeferredActions::Clear() {
        std::lock_guard<std::mutex> lock(mutex);
        actions.clear();
    }

    bool DeferredActions::HasPendingActions() {
        std::lock_guard<std::mutex> lock(mutex);
        return !actions.empty();
    }
    void DeferredActions::Process() {
        std::vector<std::function<void()>> actionsToProcess;
        {
            std::lock_guard<std::mutex> lock(mutex);
            actionsToProcess.swap(actions);
        }

        for (auto& action : actionsToProcess)
            action();
    }
}