#pragma once
#include <EGSDK\ClassHelpers.h>
#include <EGSDK\Utils\Time.h>

namespace EGSDK::GamePH {
	namespace TimeWeather {
		class CSystem;
	}

	class PlayerDI_PH;

	class EGameSDK_API LevelDI {
	public:
		bool IsLoading();
		bool IsLoaded();
		void* GetViewCamera();
		float GetTimeDelta();
		void SetViewCamera(void* viewCam);
		float GetTimePlayed();
		void ShowUIManager(bool enabled);
		bool IsTimerFrozen();
		float TimerGetSpeedUp();
		void TimerSetSpeedUp(float timeScale);
		TimeWeather::CSystem* GetTimeWeatherSystem();

		static LevelDI* Get();
	private:
		static Utils::Time::Timer loadTimer;
		static bool relyOnTimer;
		static bool hasLoaded;

		static void ResetLoadTimer();
	};
}