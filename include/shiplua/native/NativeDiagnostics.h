#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace ShipLua {

// Marca, na thread atual, que código nativo de um mod está rodando. Os ponteiros
// precisam viver até o fim do escopo (strings do provider ou do registro de hooks).
class NativeActivityScope {
  public:
    NativeActivityScope(const char* owner, const char* phase, const char* detail) noexcept;
    ~NativeActivityScope();
    NativeActivityScope(const NativeActivityScope&) = delete;
    NativeActivityScope& operator=(const NativeActivityScope&) = delete;

    const char* owner;
    const char* phase;
    const char* detail;
    NativeActivityScope* previous;
};

// Mapa endereço → mod para as bibliotecas carregadas. Sem alocação; até 64 módulos.
void RegisterNativeModule(const char* owner, void* library) noexcept;
void UnregisterNativeModule(void* library) noexcept;
// Dono do módulo que contém `address`, copiado em `owner`; false se nenhum.
bool FindNativeModuleOwner(uintptr_t address, char* owner, size_t capacity) noexcept;

// Proteção de boot: arquivo marcador escrito antes de carregar cada mod nativo.
void SetNativeBootMarker(const std::filesystem::path& marker) noexcept;
void ClearNativeBootMarker() noexcept;
// Grava "<estado>\t<mod>" no marcador ativo (loading, running, crashed).
void WriteNativeBootMarker(const char* state, const char* owner) noexcept;

// Para o crash handler, na thread que falhou, sem alocar. Escreve a atividade
// nativa da thread, os mods cujas bibliotecas aparecem na pilha e os módulos
// carregados. Se o boot ainda não foi confirmado, grava no marcador o mod
// culpado. Retorna os bytes escritos (sem NUL).
size_t WriteNativeCrashReport(char* buffer, size_t capacity) noexcept;

} // namespace ShipLua
