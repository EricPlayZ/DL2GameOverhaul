#pragma once
#include <Windows.h>
#include <EGSDK\Exports.h>

namespace EGSDK::Engine {
	class EGameSDK_API CInput {
	public:
		DWORD64 BlockGameInput();
		void UnlockGameInput();

		static CInput* Get();
	};
}