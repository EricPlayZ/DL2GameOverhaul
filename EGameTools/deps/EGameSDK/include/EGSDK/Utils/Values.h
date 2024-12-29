#pragma once
#include <string>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Values {
		extern EGameSDK_API bool str_ends_with_ci(std::string const& text, std::string const& substr);

		extern EGameSDK_API bool are_samef(float a, float b, float precision = 0.0001f);
		extern EGameSDK_API float round_decimal(float value, int decimal_places = 2);

		extern EGameSDK_API bool str_replace(std::string& str, const std::string& from, const std::string& to);
		template <typename T> auto to_string(T val) {
			if constexpr (std::is_same<T, std::string>::value)
				return static_cast<std::string>(val);
			else
				return std::to_string(val);
		}
	}
}