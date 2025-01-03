#include <string>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <EGSDK\Utils\Values.h>

namespace EGSDK::Utils {
    namespace Values {
        bool str_ends_with_ci(std::string const& text, std::string const& substr) {
            if (substr.length() > text.length())
                return false;

            auto it = std::search(text.rbegin(), text.rbegin() + substr.length(), substr.rbegin(), substr.rend(), [](char ch1, char ch2) {
                return std::toupper(ch1) == std::toupper(ch2);
            });
            return it == text.rbegin();
        }

        bool are_samef(float a, float b, float precision) { return abs(a - b) < precision; }
        float round_decimal(float value, int decimal_places) {
            const double multiplier = std::pow(10.0f, decimal_places);
            return std::roundf(value * static_cast<float>(multiplier)) / static_cast<float>(multiplier);
        }

        bool str_replace(std::string& str, const std::string& from, const std::string& to) {
            const size_t start_pos = str.find(from);
            if (start_pos == std::string::npos)
                return false;

            str.replace(start_pos, from.length(), to);
            return true;
        }

        std::string GetSimpleTypeName(std::string fullName) {
            if (fullName.compare(0, 6, "class ") == 0)
                fullName.erase(0, 6);
            else if (fullName.compare(0, 7, "struct ") == 0)
                fullName.erase(0, 7);
            else if (fullName.compare(0, 5, "enum ") == 0)
                fullName.erase(0, 5);

            size_t pos = fullName.find_last_of("::");
            if (pos != std::string::npos)
                fullName.erase(0, pos + 1);

            return fullName;
        }
    }
}