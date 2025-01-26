#include <EGT\ImGui_impl\NextFrameTask.h>

namespace EGT::ImGui_impl {
    void NextFrameTask::AddTask(const Task& task, int delayFrames) {
        std::lock_guard<std::mutex> lock(GetMutex());
        GetTaskQueue().emplace_back(TaskEntry{ delayFrames, task });
    }
    void NextFrameTask::ExecuteTasks() {
        std::lock_guard<std::mutex> lock(GetMutex());
        auto& queue = GetTaskQueue();

        for (auto it = queue.begin(); it != queue.end();) {
            if (it->framesRemaining <= 0) {
                it->task();
                it = queue.erase(it);
            } else {
                --it->framesRemaining;
                ++it;
            }
        }
    }

    std::vector<NextFrameTask::TaskEntry>& NextFrameTask::GetTaskQueue() {
        static std::vector<TaskEntry> taskQueue;
        return taskQueue;
    }
    std::mutex& NextFrameTask::GetMutex() {
        static std::mutex mutex;
        return mutex;
    }
}