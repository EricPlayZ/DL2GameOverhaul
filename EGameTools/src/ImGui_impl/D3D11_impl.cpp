﻿#include <vector>
#include <mutex>
#include <d3d11.h>
#include <kiero\kiero.h>
#include <spdlog\spdlog.h>
#include <ImGui\backends\imgui_impl_dx11.h>
#include <ImGui\backends\imgui_impl_win32.h>
#include <EGT\ImGui_impl\Win32_impl.h>
#include <EGT\ImGui_impl\DeferredActions.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Init.h>

namespace EGT::ImGui_impl {
	namespace D3D11 {
		ID3D11Device* d3d11Device = nullptr;
		static ID3D11DeviceContext* d3d11DeviceContext = nullptr;
		static ID3D11RenderTargetView* d3d11RenderTargetView = nullptr;

		static std::mutex resourceMutex{};
		static std::vector<IUnknown*> resources{};

		static void ReleaseResources() {
			resourceMutex.lock();
			for (auto resource : resources)
				resource->Release();
			resources.clear();
			d3d11RenderTargetView = nullptr;
			resourceMutex.unlock();
		}
		// Create custom render target view because DX11 color space issue with ImGui bruh
		static ID3D11RenderTargetView* CreateRenderTargetView(IDXGISwapChain* swapChain) {
			ID3D11Device* device = nullptr;
			ID3D11Texture2D* backBuffer = nullptr;
			ID3D11RenderTargetView* renderTarget = nullptr;

			swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
			swapChain->GetDevice(IID_PPV_ARGS(&device));

			if (device && backBuffer) {
				D3D11_RENDER_TARGET_VIEW_DESC desc{};
				memset(&desc, 0, sizeof(desc));
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

				if (device->CreateRenderTargetView(backBuffer, &desc, &renderTarget) == S_OK) {
					resourceMutex.lock();
					resources.push_back(renderTarget);
					resourceMutex.unlock();
				}
				backBuffer->Release();
			}
			return renderTarget;
		}

		static void RenderImGui_DX11(IDXGISwapChain* pSwapChain) {
			static bool init = false;

			if (!init) {
				DXGI_SWAP_CHAIN_DESC desc{};
				pSwapChain->GetDesc(&desc);

				pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&d3d11Device);
				d3d11Device->GetImmediateContext(&d3d11DeviceContext);

				d3d11RenderTargetView = CreateRenderTargetView(pSwapChain);

				Win32::Init(desc.OutputWindow);

				ImGui::CreateContext();
				ImGui::GetIO().IniFilename = nullptr;

				ImGui_ImplWin32_Init(desc.OutputWindow);
				ImGui_ImplDX11_Init(d3d11Device, d3d11DeviceContext);

				Menu::InitImGui();

				init = true;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			Menu::FirstTimeRunning();
			if (Menu::menuToggle.GetValue())
				Menu::Render();

			ImGui::EndFrame();
			ImGui::Render();

			DeferredActions::Process();

			if (d3d11RenderTargetView)
				d3d11DeviceContext->OMSetRenderTargets(1, &d3d11RenderTargetView, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		}

		HRESULT(__stdcall* oPresent)(IDXGISwapChain*, UINT, UINT);
		HRESULT __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
			__try {
				RenderImGui_DX11(pSwapChain);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				SPDLOG_ERROR("Exception thrown rendering ImGui in DX11");
			}

			return oPresent(pSwapChain, SyncInterval, Flags);
		}

		HRESULT(__stdcall* oResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
		HRESULT __stdcall hkResizeBuffers11(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
			ReleaseResources();
			HRESULT result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
			d3d11RenderTargetView = CreateRenderTargetView(pSwapChain);

			return result;
		}

		void Init() {
			assert(kiero::bind(8, (void**)&oPresent, hkPresent11) == kiero::Status::Success);
			assert(kiero::bind(13, (void**)&oResizeBuffers, hkResizeBuffers11) == kiero::Status::Success);
		}
	}
}