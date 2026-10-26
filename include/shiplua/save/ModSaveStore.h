#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "shiplua/runtime/Result.h"
#include "shiplua/storage/KeyValueStorage.h"

namespace ShipLua {

// RFC 0031: dados de um mod dentro do arquivo de save carregado (ship.save), com os valores de ship.storage.
using ModSaveValues = std::map<std::string, KeyValueStorage::Value>;

// Lado do host. No OoT, o namespace "<id do mod>" do linkspan.oot.save, versão 1, objeto JSON.
class ModSaveBackend {
  public:
    virtual ~ModSaveBackend() = default;
    // Há um arquivo carregado e o jogo está em gameplay.
    virtual bool Available() const = 0;
    // Bloco do mod no arquivo carregado; vazio sem bloco.
    virtual Result<ModSaveValues> Load(const std::string& modId) = 0;
    // Substitui o bloco do mod. Vale na memória e chega ao disco no próximo save do jogo.
    virtual Result<void> Store(const std::string& modId, const ModSaveValues& values) = 0;
};

struct ModSaveLimits {
    std::size_t maxKeys = 256;
    std::size_t maxKeyBytes = 128;
    std::size_t maxValueBytes = 4096;      // texto conta os bytes; booleano e número contam 8
    std::size_t maxTotalBytes = 64 * 1024; // soma de chaves e valores
};

// Validação e limites de ship.save sobre um backend. Sem cache: cada chamada lê do backend, então trocar de arquivo
// nunca mostra dados do anterior. Só na thread do jogo.
class ModSaveStore {
  public:
    explicit ModSaveStore(std::shared_ptr<ModSaveBackend> backend, ModSaveLimits limits = {});

    bool Available() const;
    // InvalidState sem arquivo; InvalidArgument com chave vazia ou maior que o limite.
    Result<std::optional<KeyValueStorage::Value>> Get(const std::string& modId, const std::string& key);
    // InvalidState sem arquivo; InvalidArgument com chave inválida ou número não finito; ResourceLimit além dos limites.
    Result<void> Set(const std::string& modId, const std::string& key, KeyValueStorage::Value value);
    // true quando a chave existia.
    Result<bool> Delete(const std::string& modId, const std::string& key);

    const ModSaveLimits& Limits() const;

  private:
    std::shared_ptr<ModSaveBackend> mBackend;
    ModSaveLimits mLimits;
};

// Backend em memória do mock e dos testes. LoadSlot/Unload simulam abrir e sair de um arquivo; cada slot guarda os
// próprios blocos.
class MemoryModSaveBackend final : public ModSaveBackend {
  public:
    bool Available() const override;
    Result<ModSaveValues> Load(const std::string& modId) override;
    Result<void> Store(const std::string& modId, const ModSaveValues& values) override;

    void LoadSlot(int slot);
    void Unload();
    int Slot() const; // -1 sem arquivo

  private:
    int mSlot = -1;
    std::map<int, std::map<std::string, ModSaveValues>> mSlots;
};

} // namespace ShipLua
