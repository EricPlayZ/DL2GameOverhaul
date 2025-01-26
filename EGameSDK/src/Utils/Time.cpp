#include <EGSDK\Utils\Time.h>

namespace EGSDK::Utils {
    namespace Time {
        Timer::Timer(int64_t timeMs) : timeToPass(timeMs), timePassed(false) {
            const auto currentClock = clock::now();
            start = std::chrono::duration_cast<std::chrono::milliseconds>(currentClock.time_since_epoch()).count();
            end = start + timeToPass;
        }

        const int64_t Timer::GetTimePassed() const {
            if (timePassed)
                return -1;

            const auto currentClock = clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentClock.time_since_epoch());

            return duration.count() - start;
        }
        const bool Timer::DidTimePass() {
            if (timePassed)
                return true;

            const auto currentClock = clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentClock.time_since_epoch());

            if (duration.count() >= end)
                timePassed = true;
            return timePassed;
        }
        void Timer::Reset() {
            *this = Timer(timeToPass);
        }

        std::ostringstream GetLogTimestamp() {
            time_t timeInstance = time(0);
            tm timestamp{};
            localtime_s(&timestamp, &timeInstance);

            std::ostringstream oss{};
            oss << "[" << std::setw(2) << std::setfill('0') << timestamp.tm_hour << "h:"
                << std::setw(2) << std::setfill('0') << timestamp.tm_min << "m:"
                << std::setw(2) << std::setfill('0') << timestamp.tm_sec << "s] ";

            return oss;
        }
    }
}