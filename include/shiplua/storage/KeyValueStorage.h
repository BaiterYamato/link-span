#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "shiplua/runtime/Result.h"

namespace ShipLua {

// Armazenamento chave-valor por mod, em memória, com namespace automático por
// mod e limites de segurança (plan-sdk.md §8.18: namespace por mod, limite de
// tamanho, tipos serializáveis restritos). Valores aceitos são os tipos
// serializáveis do contrato: booleano, inteiro, número e texto.
//
// Determinístico (std::map) e confinado à thread dona, seguindo o padrão de
// EventDispatcher e FrameTimerScheduler. O mock runtime usa esta implementação
// diretamente; hosts reais podem persistir o mesmo conteúdo via AtomicFile.
class KeyValueStorage {
  public:
    using Value = std::variant<bool, std::int64_t, double, std::string>;

    struct Limits {
        std::size_t maxKeysPerMod = 1024;
        std::size_t maxKeyBytes = 128;
        std::size_t maxValueBytes = 64 * 1024;
        std::size_t maxTotalBytesPerMod = 256 * 1024; // plan-sdk.md: storage_kb = 256
    };

    KeyValueStorage() = default;
    explicit KeyValueStorage(Limits limits);

    KeyValueStorage(const KeyValueStorage&) = delete;
    KeyValueStorage& operator=(const KeyValueStorage&) = delete;

    const Limits& GetLimits() const;

    // Retorna o valor da chave ou std::nullopt quando ausente.
    Result<std::optional<Value>> Get(const std::string& modId, const std::string& key) const;

    // Grava o valor. ResourceLimit quando os limites do mod são excedidos.
    Result<void> Set(const std::string& modId, const std::string& key, Value value);

    // Remove a chave; retorna true quando ela existia.
    Result<bool> Delete(const std::string& modId, const std::string& key);

    // Remove todas as chaves do mod; retorna quantas foram removidas.
    Result<std::size_t> Clear(const std::string& modId);

    // Lista as chaves do mod em ordem lexicográfica determinística.
    Result<std::vector<std::string>> Keys(const std::string& modId) const;

    std::size_t KeyCount(const std::string& modId) const;
    std::size_t TotalBytes(const std::string& modId) const;

    // Serializa todo o conteúdo (todos os mods) num blob binário determinístico
    // e versionado. Length-prefixed little-endian; ver KeyValueStorage.cpp.
    std::string Serialize() const;

    // Substitui o conteúdo atual pelo do blob. Erro se o blob estiver corrompido.
    // Entradas que violem os limites do mod são puladas (não derrubam a carga
    // inteira), preservando o resto — um save antigo/maior nunca trava o mod.
    Result<void> Deserialize(std::string_view blob);

    // Liga persistência em disco neste store: carrega o arquivo existente (se
    // houver) e passa a MARCAR mudanças para gravação posterior.
    // Arquivo ausente NÃO é erro (primeira execução). Chamar uma vez no init,
    // antes de os mods rodarem.
    //
    // A gravação é ADIADA de propósito. Gravar a cada Set custa serializar o
    // store inteiro mais um write+rename síncronos; um mod que atualize alguns
    // contadores por segundo trava visivelmente o jogo, e mais ainda quando um
    // antivírus segura o arquivo (o rename do AtomicFile chega a dormir dezenas
    // de milissegundos tentando de novo). O host chama Flush() num intervalo
    // folgado e no shutdown.
    Result<void> EnablePersistence(std::filesystem::path path);

    // Grava agora, se houver mudança pendente e a persistência estiver ligada.
    // No-op barato caso contrário — pode ser chamado por frame sem custo.
    Result<void> Flush();

    // true quando há mudança ainda não gravada.
    bool IsDirty() const;

  private:
    Result<void> ValidateKey(const std::string& modId, const std::string& key) const;
    static std::size_t ValueBytes(const Value& value);
    // Marca que há mudança a gravar. Chamado ao fim de Set/Delete/Clear.
    void MarkDirty();
    // Remove temporários órfãos (<arquivo>.shiplua-tmp-*) ao lado do destino.
    // Um rename que falhe com o arquivo travado por outro processo pode deixar
    // o temporário para trás; sem isto eles se acumulam na pasta do jogo.
    void SweepStaleTemporaries() const;

    Limits mLimits;
    std::map<std::string, std::map<std::string, Value>> mMods;
    std::map<std::string, std::size_t> mBytesPerMod;
    std::optional<std::filesystem::path> mPersistPath;
    bool mDirty = false;
};

} // namespace ShipLua
