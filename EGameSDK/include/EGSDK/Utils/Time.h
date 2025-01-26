#pragma once
#include <stdint.h>
#include <chrono>
#include <iomanip>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Time {
		class EGameSDK_API Timer {
			using clock = std::chrono::steady_clock;
		public:
			int64_t timeToPass;
			Timer() : timeToPass(0), timePassed(false), start(0), end(0) {}
			Timer(int64_t timeMs);

			const int64_t GetTimePassed() const;
			const bool DidTimePass();
			void Reset();
		private:
			int64_t start;
			int64_t end;
			bool timePassed;
		};

		template <class result_t = std::chrono::milliseconds, class clock_t = std::chrono::steady_clock, class duration_t = std::chrono::milliseconds>
		auto since(std::chrono::time_point<clock_t, duration_t> const& start) {
			return std::chrono::duration_cast<result_t>(clock_t::now() - start);
		}

		extern EGameSDK_API std::ostringstream GetLogTimestamp();
	}
}