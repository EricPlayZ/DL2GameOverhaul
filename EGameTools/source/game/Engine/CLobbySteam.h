#pragma once
#include "..\ClassHelpers.h"

namespace Engine {
	class CGame;

	class CLobbySteam {
	public:
		union {
			buffer<0xF8, CGame*> pCGame;
		};

		static CLobbySteam* Get();
	};
}