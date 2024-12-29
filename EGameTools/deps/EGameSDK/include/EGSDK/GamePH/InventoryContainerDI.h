#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <EGSDK\GamePH\InventoryContainerDI.h>
#include <EGSDK\GamePH\InventoryMoney.h>
#include <EGSDK\ClassHelpers.h>

namespace EGSDK::GamePH {
	class EGameSDK_API InventoryContainerDI {
	public:
		InventoryMoney* GetInventoryMoney(UINT indexMaybe);
	};
}