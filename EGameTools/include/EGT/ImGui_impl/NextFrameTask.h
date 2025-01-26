#pragma once
#include <functional>
#include <vector>
#include <mutex>

namespace EGT::ImGui_impl {
    class NextFrameTask {
        using Task = std::function<void()>;

    public:
        static void AddTask(const Task& task, int delayFrames = 1);
        static void ExecuteTasks();
    private:
        struct TaskEntry {
            int framesRemaining = 1;
            Task task{};
        };

        static std::vector<TaskEntry>& GetTaskQueue();
        static std::mutex& GetMutex();
    };
}