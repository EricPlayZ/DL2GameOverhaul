#pragma once
#include <Windows.h>
#include <string_view>
#include <vector>
#include <EGSDK\Exports.h>

namespace EGSDK::Utils {
	namespace SigScan {
		enum PatternType {
			Address,
			Pointer,
			PointerBYTE,
			PointerWORD,
			PointerDWORD,
			PointerQWORD,
			RelativePointer,
			RelativePointerBYTE,
			RelativePointerWORD,
			RelativePointerDWORD,
			RelativePointerQWORD,
		};

		struct EGameSDK_API Pattern {
			const char* pattern;
			PatternType type;
		};

		class EGameSDK_API PatternScanner {
		public:
			static void* FindPattern(void* startAddress, DWORD64 searchSize, const Pattern& pattern);
			static void* FindPattern(const std::string_view moduleName, const Pattern& pattern);
			static void* FindPattern(const Pattern& pattern);

			static std::vector<void*> FindPatterns(const std::string_view moduleName, const Pattern& pattern);
			static std::vector<void*> FindPatterns(void* startAddress, DWORD64 searchSize, const Pattern& pattern);

			static void* FindPattern(void* startAddress, DWORD64 searchSize, const Pattern* patterns, float* ratio = nullptr);
			static void* FindPattern(const std::string_view moduleName, Pattern* patterns, float* ratio = nullptr);
		private:
			template <typename T> static void* ResolveRelativePtr(void* Address) {
				if (!Address)
					return nullptr;

				T offset = *reinterpret_cast<T*>(Address);
				if (!offset)
					return nullptr;

				return reinterpret_cast<void*>(reinterpret_cast<DWORD64>(Address) + offset + sizeof(T));
			}

			template <typename T> static void* ResolvePtr(void* Address) {
				if (!Address)
					return nullptr;

				return reinterpret_cast<void*>(*reinterpret_cast<T*>(Address));
			}
		};
	}
}