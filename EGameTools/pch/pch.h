#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <any>
#include <filesystem>
#include <exception>
#include <ctype.h>
#include <cfloat>
#include <cstring>
#include <cstdarg>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <thread>
#include <complex>
#include <list>
#define _USE_MATH_DEFINES
#include <cmath>
#include <assert.h>
#include <unordered_map>
#include <unordered_set>

#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3d12.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <spdlog\spdlog.h>
#include <spdlog\sinks\rotating_file_sink.h>
#include <backends\imgui_impl_dx11.h>
#include <backends\imgui_impl_dx12.h>
#include <backends\imgui_impl_win32.h>
#include <impl\d3d11_impl.h>
#include <impl\d3d12_impl.h>
#include <impl\win32_impl.h>
#include <misc\dirent\dirent.h>
#include <misc\fonts\Ruda-Bold.embed>
#include <ImGuiFileDialog\ImGuiFileDialog.h>
#include <ImGuiFileDialog\ImGuiFileDialogConfig.h>
#include <imconfig.h>
#include <imgui_hotkey.h>
#include <imgui_internal.h>
#include <imguiex.h>
#include <imguiex_animation.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <MinHook.h>

#include "..\source\config\ini.h"
#include "..\source\game\Engine\GameSpeedHandler.h"
#include "..\source\game\Vector3.h"
#include "..\source\game\buffer.h"
#include "..\source\utils\files.h"
#include "..\source\utils\hook.h"
#include "..\source\utils\memory.h"
#include "..\source\utils\sigscan.h"
#include "..\source\utils\texture.h"
#include "..\source\utils\time.h"
#include "..\source\utils\values.h"
#include "..\source\utils\windows.h"
#include "..\source\kiero.h"

#include "..\textures\EGTWhiteLogo.embed"