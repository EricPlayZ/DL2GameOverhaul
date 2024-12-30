#include <MinHook.h>

#include "FunctionHookMinHook.hpp"

using namespace std;

bool g_isMinHookInitialized{ false };

FunctionHookMinHook::FunctionHookMinHook(uintptr_t target, uintptr_t destination)
    : m_target{ 0 },
    m_destination{ 0 },
    m_original{ 0 }
{
    // Initialize MinHook if it hasn't been already.
    if (!g_isMinHookInitialized && MH_Initialize() == MH_OK) {
        g_isMinHookInitialized = true;
    }

    // Create the hook. Call create afterwards to prevent race conditions accessing FunctionHookMinHook before it leaves its constructor.
    if (auto status = MH_CreateHook(reinterpret_cast<void*>(target), reinterpret_cast<void*>(destination), (LPVOID*)&m_original); status == MH_OK) {
        m_target = target;
        m_destination = destination;
    }
}

FunctionHookMinHook::~FunctionHookMinHook() {
    remove();
}

bool FunctionHookMinHook::create() {
    if (m_target == 0 || m_destination == 0 || m_original == 0) {
        return false;
    }

    if (auto status = MH_EnableHook((LPVOID)m_target); status != MH_OK) {
        m_original = 0;
        m_destination = 0;
        m_target = 0;

        return false;
    }

    return true;
}

bool FunctionHookMinHook::remove() {
    // Don't try to remove invalid hooks.
    if (m_original == 0) {
        return true;
    }

    // Disable then remove the hook.
    if (MH_DisableHook((LPVOID)m_target) != MH_OK ||
        MH_RemoveHook((LPVOID)m_target) != MH_OK) {
        return false;
    }

    // Invalidate the members.
    m_target = 0;
    m_destination = 0;
    m_original = 0;

    return true;
}