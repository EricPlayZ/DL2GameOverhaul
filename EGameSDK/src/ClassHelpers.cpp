#include <EGSDK\ClassHelpers.h>

namespace EGSDK {
	namespace ClassHelpers {
        std::string GetFieldNameFromRTTI(std::string fullName) {
            size_t arrayStart = fullName.find("std::array<char,");
            if (arrayStart == std::string::npos)
                return "";

            size_t braceStart = fullName.find('{', arrayStart);
            if (braceStart == std::string::npos)
                return "";

            size_t secondBrace = fullName.find('{', braceStart + 1);
            if (secondBrace == std::string::npos)
                return "";

            size_t braceEnd = fullName.find('}', secondBrace + 1);
            if (braceEnd == std::string::npos)
                return "";

            // Extract the array content (e.g., "112,67,76,101,118,101,108,0")
            std::string arrayContent = fullName.substr(secondBrace + 1, braceEnd - secondBrace - 1);

            // Parse the array content into a string
            std::stringstream ss(arrayContent);
            std::string token{};
            std::string result{};
            while (std::getline(ss, token, ',')) {
                int charCode = std::stoi(token); // Convert each number to an integer
                if (charCode == 0)              // Stop at the null terminator
                    break;
                result += static_cast<char>(charCode); // Append the character to the result
            }

            return result;
        }

		static volatile LONG disableVftableScanning = 0;

		bool IsVftableScanningDisabled() { return _InterlockedCompareExchange(&disableVftableScanning, 0, 0) != 0; }
		void SetIsVftableScanningDisabled(bool value) { _InterlockedExchange(&disableVftableScanning, value ? 1 : 0); }
	}
}