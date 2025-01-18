#pragma once
#include <EGSDK\exports.h>
#include <steam\steam_api.h>

namespace EGSDK::Core {
	class EGameSDK_API SteamAPI {
	public:
		static ISteamClient* SteamClient();
		static HSteamUser GetHSteamUser();
		static HSteamPipe GetHSteamPipe();

		static ISteamUser* GetISteamUser();
		static ISteamUtils* GetISteamUtils();
	};
}