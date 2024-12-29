#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
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