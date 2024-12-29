#pragma once
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace Windows {
		enum class WindowsVersion {
			Unknown,
			Windows7,
			Windows10
		};

		extern EGameSDK_API const WindowsVersion GetWindowsVersion();
	}
}