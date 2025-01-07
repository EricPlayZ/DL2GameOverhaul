#include <ImGui\imgui_hotkey.h>
#include <ImGui\imguiex.h>
#include <EGSDK\GamePH\LevelDI.h>
#include <EGSDK\GamePH\PlayerDI_PH.h>
#include <EGSDK\GamePH\PlayerVariables.h>
#include <EGT\Menu\Menu.h>
#include <EGT\Menu\Weapon.h>

namespace EGT::Menu {
	namespace Weapon {
		float currentWeaponDurability = 0.0f;
		ImGui::KeyBindOption unlimitedDurability{ VK_NONE };
		ImGui::KeyBindOption unlimitedAmmo{ VK_NONE };
		ImGui::KeyBindOption noSpread{ VK_NONE };
		ImGui::KeyBindOption noRecoil{ VK_NONE };
		ImGui::KeyBindOption instantReload{ VK_NONE };

		static constexpr float baseWeaponDurabilityMul = 1.0f;
		static constexpr float baseWeaponAccuracyMul = 1.0f;
		static constexpr float baseBowAccuracyMul = 0.1f;
		static constexpr float baseWeaponRecoilMul = 1.0f;
		static constexpr float baseWeaponReloadMul = 1.0f;

		static void PlayerVarsUpdate() {
			if (!EGSDK::GamePH::PlayerVariables::gotPlayerVars)
				return;

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("MeleeWpnDurabilityMulReduce", 0.0f, baseWeaponDurabilityMul, unlimitedDurability.GetValue(), true);

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsPistolInfiniteAmmo", true, false, unlimitedAmmo.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRevolverInfiniteAmmo", true, false, unlimitedAmmo.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRifleInfiniteAmmo", true, false, unlimitedAmmo.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsShotgunInfiniteAmmo", true, false, unlimitedAmmo.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsSMGInfiniteAmmo", true, false, unlimitedAmmo.GetValue());
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("InfiniteArrows", true, false, unlimitedAmmo.GetValue());

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BulletAccuracyFactor", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsMoveAccuracyReduce", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsPistolAccuracyFactor", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRevolverAccuracyFactor", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRifleAccuracyFactor", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsShotgunAccuracyFactor", 0.0f, baseWeaponAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsCrossbowAccuracyFactor", 0.0f, baseBowAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsHarpoonAccuracyFactor", 0.0f, baseBowAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BowAccuracyFactor", 0.0f, baseBowAccuracyMul, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BowMaxThrowFactor", 99999.0f, 1.0f, noSpread.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BowSlowMoAccuracyMul", 0.0f, 0.25f, noSpread.GetValue(), true);

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BulletRecoilFactor", 0.0f, baseWeaponRecoilMul, noRecoil.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsPistolRecoilFactor", 0.0f, baseWeaponRecoilMul, noRecoil.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRevolverRecoilFactor", 0.0f, baseWeaponRecoilMul, noRecoil.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRifleRecoilFactor", 0.0f, baseWeaponRecoilMul, noRecoil.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsShotgunRecoilFactor", 0.0f, baseWeaponRecoilMul, noRecoil.GetValue(), true);

			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BulletReloadSpeed", 1000.0f, 0.0f, instantReload.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsPistolReloadTimeMul", 1000.0f, baseWeaponReloadMul, instantReload.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRevolverReloadTimeMul", 1000.0f, baseWeaponReloadMul, instantReload.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsRifleReloadTimeMul", 1000.0f, baseWeaponReloadMul, instantReload.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("FirearmsShotgunReloadTimeMul", 1000.0f, baseWeaponReloadMul, instantReload.GetValue(), true);
			EGSDK::GamePH::PlayerVariables::ManagePlayerVarByBool("BowPutArrowDuration", 0.0f, 0.137f, instantReload.GetValue(), true);
		}
		static void UpdateWeaponDurability(bool updateSlider) {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return;
			EGSDK::GamePH::PlayerDI_PH* player = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!player)
				return;
			EGSDK::GamePH::InventoryItem* weaponItem = player->GetCurrentWeapon(0);
			if (!weaponItem)
				return;
			EGSDK::GamePH::ItemDescWithContext* weaponItemCtx = weaponItem->GetItemDescCtx();
			if (!weaponItemCtx)
				return;

			updateSlider ? (currentWeaponDurability = weaponItemCtx->weaponDurability) : (weaponItemCtx->weaponDurability = currentWeaponDurability);
		}

		static bool isWeaponInteractionDisabled() {
			EGSDK::GamePH::LevelDI* iLevel = EGSDK::GamePH::LevelDI::Get();
			if (!iLevel || !iLevel->IsLoaded())
				return true;
			EGSDK::GamePH::PlayerDI_PH* player = EGSDK::GamePH::PlayerDI_PH::Get();
			if (!player)
				return true;
			EGSDK::GamePH::InventoryItem* weaponItem = player->GetCurrentWeapon(0);
			if (!weaponItem || !weaponItem->GetItemDescCtx())
				return true;

			return false;
		}

		Tab Tab::instance{};
		void Tab::Update() {
			if (!Menu::menuToggle.GetValue())
				UpdateWeaponDurability(true);
			PlayerVarsUpdate();
		}
		void Tab::Render() {
			ImGui::SeparatorText("Current Weapon##Weapon");
			ImGui::BeginDisabled(isWeaponInteractionDisabled() || currentWeaponDurability <= 0.0f); {
				if (ImGui::SliderFloat("Weapon Durability", "Currently only works while your weapon is physically equipped in your hand", &currentWeaponDurability, 0.1f, 999.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					UpdateWeaponDurability(false);
				else
					UpdateWeaponDurability(true);
				ImGui::EndDisabled();
			}

			ImGui::SeparatorText("Misc##Weapon");
			ImGui::CheckboxHotkey("Unlimited Durability", &unlimitedDurability, "Enables unlimited durability for all weapons");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("Unlimited Ammo", &unlimitedAmmo, "Enables unlimited ammo for all firearms and bows");
			ImGui::CheckboxHotkey("No Spread", &noSpread, "Disables random bullet spread for all firearms and bows (doesn't completely remove spread for bows)");
			ImGui::SameLine();
			ImGui::CheckboxHotkey("No Recoil", &noRecoil, "Disables weapon recoil for all firearms");
			ImGui::CheckboxHotkey("Instant Reload", &instantReload, "Makes reloading firearms and bows (almost) instant");
		}
	}
}