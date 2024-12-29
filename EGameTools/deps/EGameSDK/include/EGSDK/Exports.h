#pragma once
#ifdef EGameSDK_EXPORTS
#define EGameSDK_API __declspec(dllexport)
#else
#define EGameSDK_API __declspec(dllimport)
#endif