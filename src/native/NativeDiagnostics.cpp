#include "shiplua/native/NativeDiagnostics.h"

#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace ShipLua {
namespace {

thread_local NativeActivityScope* gCurrent = nullptr;

struct ModuleSlot {
    std::atomic<uintptr_t> base{0};
    std::atomic<uintptr_t> size{0};
    void* library = nullptr;
    char owner[128] = {};
};

std::array<ModuleSlot, 64>& Modules() {
    static std::array<ModuleSlot, 64> modules;
    return modules;
}

// Caminho do marcador de boot, legível pelo crash handler sem alocação.
std::atomic<bool> gMarkerActive{false};
#ifdef _WIN32
wchar_t gMarkerPath[1024] = {};
#else
char gMarkerPath[1024] = {};
#endif

void CopyText(char* target, size_t capacity, const char* source) noexcept {
    if (!capacity) return;
    size_t index = 0;
    for (; source && source[index] && index + 1 < capacity; ++index) target[index] = source[index];
    target[index] = '\0';
}

struct Writer {
    char* buffer;
    size_t capacity;
    size_t used = 0;
    void Text(const char* text) noexcept {
        for (size_t index = 0; text && text[index] && used < capacity; ++index) buffer[used++] = text[index];
    }
    void Line(const char* text) noexcept {
        Text(text);
        Text("\n");
    }
};

void WriteMarker(const char* state, const char* owner) noexcept {
    if (!gMarkerActive.load()) return;
#ifdef _WIN32
    HANDLE file = CreateFileW(gMarkerPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    char text[192];
    const int length = std::snprintf(text, sizeof(text), "%s\t%s\n", state, owner ? owner : "");
    DWORD written = 0;
    if (length > 0) WriteFile(file, text, static_cast<DWORD>(length), &written, nullptr);
    FlushFileBuffers(file);
    CloseHandle(file);
#else
    FILE* file = std::fopen(gMarkerPath, "wb");
    if (!file) return;
    std::fprintf(file, "%s\t%s\n", state, owner ? owner : "");
    std::fclose(file);
#endif
}

} // namespace

NativeActivityScope::NativeActivityScope(const char* ownerId, const char* phaseName, const char* detailText) noexcept
    : owner(ownerId), phase(phaseName), detail(detailText), previous(gCurrent) {
    gCurrent = this;
}

NativeActivityScope::~NativeActivityScope() {
    gCurrent = previous;
}

void RegisterNativeModule(const char* owner, void* library) noexcept {
#ifdef _WIN32
    MODULEINFO info{};
    if (!library || !GetModuleInformation(GetCurrentProcess(), static_cast<HMODULE>(library), &info, sizeof(info))) {
        return;
    }
    for (auto& slot : Modules()) {
        uintptr_t expected = 0;
        if (slot.base.load() == 0 && slot.size.compare_exchange_strong(expected, info.SizeOfImage)) {
            CopyText(slot.owner, sizeof(slot.owner), owner);
            slot.library = library;
            slot.base.store(reinterpret_cast<uintptr_t>(info.lpBaseOfDll));
            return;
        }
    }
#else
    (void)owner;
    (void)library;
#endif
}

void UnregisterNativeModule(void* library) noexcept {
    for (auto& slot : Modules()) {
        if (slot.library == library && slot.base.load() != 0) {
            slot.base.store(0);
            slot.library = nullptr;
            slot.owner[0] = '\0';
            slot.size.store(0);
        }
    }
}

bool FindNativeModuleOwner(uintptr_t address, char* owner, size_t capacity) noexcept {
    for (const auto& slot : Modules()) {
        const auto base = slot.base.load();
        const auto size = slot.size.load();
        if (base && address >= base && address < base + size) {
            CopyText(owner, capacity, slot.owner);
            return true;
        }
    }
    return false;
}

void SetNativeBootMarker(const std::filesystem::path& marker) noexcept {
    try {
#ifdef _WIN32
        const auto text = marker.wstring();
#else
        const auto text = marker.string();
#endif
        if (text.size() + 1 > std::size(gMarkerPath)) return;
        std::copy(text.begin(), text.end(), gMarkerPath);
        gMarkerPath[text.size()] = 0;
        gMarkerActive.store(true);
    } catch (...) {
        gMarkerActive.store(false);
    }
}

void ClearNativeBootMarker() noexcept {
    gMarkerActive.store(false);
}

size_t WriteNativeCrashReport(char* buffer, size_t capacity) noexcept {
    Writer out{buffer, capacity};
    char line[512];
    char activityBlame[128] = {};
    char stackBlame[128] = {};
    out.Line("Link-Span native:");
    if (!gCurrent) {
        out.Line("  Atividade nativa: nenhuma nesta thread");
    }
    for (auto* scope = gCurrent; scope; scope = scope->previous) {
        std::snprintf(line, sizeof(line), "  Atividade: mod '%s' em %s%s%s", scope->owner ? scope->owner : "?",
                      scope->phase ? scope->phase : "?", scope->detail ? " " : "", scope->detail ? scope->detail : "");
        out.Line(line);
        if (!activityBlame[0] && scope->owner) CopyText(activityBlame, sizeof(activityBlame), scope->owner);
    }
#ifdef _WIN32
    void* frames[62] = {};
    const USHORT count = RtlCaptureStackBackTrace(0, 62, frames, nullptr);
    char owner[128];
    char previous[128] = {};
    for (USHORT index = 0; index < count; ++index) {
        if (!FindNativeModuleOwner(reinterpret_cast<uintptr_t>(frames[index]), owner, sizeof(owner))) continue;
        if (std::strcmp(owner, previous) == 0) continue;
        CopyText(previous, sizeof(previous), owner);
        std::snprintf(line, sizeof(line), "  Pilha: biblioteca do mod '%s' (frame %u)", owner, index);
        out.Line(line);
        // O frame mais interno numa DLL de mod tem prioridade sobre a atividade registrada.
        if (!stackBlame[0]) CopyText(stackBlame, sizeof(stackBlame), owner);
    }
#endif
    for (const auto& slot : Modules()) {
        const auto base = slot.base.load();
        if (!base) continue;
        std::snprintf(line, sizeof(line), "  Modulo: %s em 0x%llx (+0x%llx)", slot.owner,
                      static_cast<unsigned long long>(base), static_cast<unsigned long long>(slot.size.load()));
        out.Line(line);
    }
    const char* blame = stackBlame[0] ? stackBlame : activityBlame;
    if (blame[0] && gMarkerActive.load()) {
        WriteMarker("crashed", blame);
        std::snprintf(line, sizeof(line), "  Boot nao confirmado: '%s' sera desativado no proximo inicio", blame);
        out.Line(line);
    }
    return out.used;
}

void WriteNativeBootMarker(const char* state, const char* owner) noexcept {
    WriteMarker(state, owner);
}

} // namespace ShipLua
