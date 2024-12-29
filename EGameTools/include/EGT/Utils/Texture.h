#pragma once
#include <imgui.h>

namespace EGT::Utils {
	namespace Texture {
		extern ImTextureID LoadImGuiTexture(const unsigned char* rawData, const int rawSize);
	}
}