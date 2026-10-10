#include "shiplua/storage/KeyValueStorage.h"

#include "shiplua/storage/AtomicFile.h"

#include <cstring>
#include <fstream>
#include <utility>

namespace ShipLua {
namespace {

// Formato do blob: magia "SLKV" + versão u8 = 1; depois u32 count de mods e,
// por mod, u32 len+bytes do modId, u32 count de entradas e, por entrada, u32
// len+bytes da chave, u8 tag de tipo (0=bool 1=int64 2=double 3=string) e o
// valor. Inteiros em little-endian explícito para ser determinístico e não
// depender da endianness da plataforma.
constexpr char kMagic[4] = {'S', 'L', 'K', 'V'};
constexpr std::uint8_t kVersion = 1;

void PutU32(std::string& out, std::uint32_t value) {
    out.push_back(static_cast<char>(value & 0xFF));
    out.push_back(static_cast<char>((value >> 8) & 0xFF));
    out.push_back(static_cast<char>((value >> 16) & 0xFF));
    out.push_back(static_cast<char>((value >> 24) & 0xFF));
}

void PutU64(std::string& out, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        out.push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
    }
}

void PutBytes(std::string& out, std::string_view bytes) {
    PutU32(out, static_cast<std::uint32_t>(bytes.size()));
    out.append(bytes.data(), bytes.size());
}

// Cursor de leitura com verificação de limites — qualquer leitura além do fim
// marca `ok=false` e as leituras seguintes viram no-op, então um blob truncado
// nunca lê lixo além do buffer.
struct Reader {
    std::string_view data;
    std::size_t pos = 0;
    bool ok = true;

    bool Take(std::size_t n, const char*& out) {
        if (!ok || pos + n > data.size()) {
            ok = false;
            return false;
        }
        out = data.data() + pos;
        pos += n;
        return true;
    }
    std::uint32_t U32() {
        const char* p = nullptr;
        if (!Take(4, p)) {
            return 0;
        }
        return static_cast<std::uint8_t>(p[0]) | (static_cast<std::uint8_t>(p[1]) << 8) |
               (static_cast<std::uint8_t>(p[2]) << 16) |
               (static_cast<std::uint32_t>(static_cast<std::uint8_t>(p[3])) << 24);
    }
    std::uint64_t U64() {
        const char* p = nullptr;
        if (!Take(8, p)) {
            return 0;
        }
        std::uint64_t value = 0;
        for (int i = 0; i < 8; ++i) {
            value |= static_cast<std::uint64_t>(static_cast<std::uint8_t>(p[i])) << (i * 8);
        }
        return value;
    }
    std::uint8_t U8() {
        const char* p = nullptr;
        return Take(1, p) ? static_cast<std::uint8_t>(p[0]) : 0;
    }
    std::string Bytes() {
        const std::uint32_t len = U32();
        const char* p = nullptr;
        if (!Take(len, p)) {
            return {};
        }
        return std::string(p, len);
    }
};

} // namespace

KeyValueStorage::KeyValueStorage(Limits limits) : mLimits(limits) {
    if (mLimits.maxKeysPerMod == 0) {
        mLimits.maxKeysPerMod = 1;
    }
    if (mLimits.maxKeyBytes == 0) {
        mLimits.maxKeyBytes = 1;
    }
}

const KeyValueStorage::Limits& KeyValueStorage::GetLimits() const {
    return mLimits;
}

std::size_t KeyValueStorage::ValueBytes(const Value& value) {
    return std::visit(
        [](const auto& item) -> std::size_t {
            using T = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return item.size();
            } else {
                return sizeof(item);
            }
        },
        value);
}

Result<void> KeyValueStorage::ValidateKey(const std::string& modId, const std::string& key) const {
    if (modId.empty()) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage requires a mod id");
    }
    if (key.empty()) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage key cannot be empty");
    }
    if (key.size() > mLimits.maxKeyBytes) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage key exceeds the size limit");
    }
    if (key.find('\0') != std::string::npos) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "storage key cannot contain NUL bytes");
    }
    return Result<void>::ok();
}

Result<std::optional<KeyValueStorage::Value>> KeyValueStorage::Get(const std::string& modId,
                                                                   const std::string& key) const {
    const auto valid = ValidateKey(modId, key);
    if (!valid.isOk()) {
        return Result<std::optional<Value>>::err(valid.code, valid.message);
    }
    const auto mod = mMods.find(modId);
    if (mod == mMods.end()) {
        return Result<std::optional<Value>>::ok(std::nullopt);
    }
    const auto entry = mod->second.find(key);
    if (entry == mod->second.end()) {
        return Result<std::optional<Value>>::ok(std::nullopt);
    }
    return Result<std::optional<Value>>::ok(entry->second);
}

Result<void> KeyValueStorage::Set(const std::string& modId, const std::string& key, Value value) {
    const auto valid = ValidateKey(modId, key);
    if (!valid.isOk()) {
        return valid;
    }
    const std::size_t valueBytes = ValueBytes(value);
    if (valueBytes > mLimits.maxValueBytes) {
        return Result<void>::err(ErrorCode::ResourceLimit,
                                 "storage value exceeds the per-key size limit");
    }

    auto& entries = mMods[modId];
    std::size_t& totalBytes = mBytesPerMod[modId];
    const auto existing = entries.find(key);
    const std::size_t previousBytes =
        existing != entries.end() ? key.size() + ValueBytes(existing->second) : 0;
    const std::size_t newBytes = key.size() + valueBytes;

    if (existing == entries.end() && entries.size() >= mLimits.maxKeysPerMod) {
        return Result<void>::err(ErrorCode::ResourceLimit,
                                 "storage key count limit reached for this mod");
    }
    if (totalBytes - previousBytes + newBytes > mLimits.maxTotalBytesPerMod) {
        return Result<void>::err(ErrorCode::ResourceLimit,
                                 "storage total size limit reached for this mod");
    }

    totalBytes = totalBytes - previousBytes + newBytes;
    entries[key] = std::move(value);
    MarkDirty();
    return Result<void>::ok();
}

Result<bool> KeyValueStorage::Delete(const std::string& modId, const std::string& key) {
    const auto valid = ValidateKey(modId, key);
    if (!valid.isOk()) {
        return Result<bool>::err(valid.code, valid.message);
    }
    const auto mod = mMods.find(modId);
    if (mod == mMods.end()) {
        return Result<bool>::ok(false);
    }
    const auto entry = mod->second.find(key);
    if (entry == mod->second.end()) {
        return Result<bool>::ok(false);
    }
    mBytesPerMod[modId] -= key.size() + ValueBytes(entry->second);
    mod->second.erase(entry);
    if (mod->second.empty()) {
        mMods.erase(mod);
        mBytesPerMod.erase(modId);
    }
    MarkDirty();
    return Result<bool>::ok(true);
}

Result<std::size_t> KeyValueStorage::Clear(const std::string& modId) {
    if (modId.empty()) {
        return Result<std::size_t>::err(ErrorCode::InvalidArgument, "storage requires a mod id");
    }
    const auto mod = mMods.find(modId);
    if (mod == mMods.end()) {
        return Result<std::size_t>::ok(0);
    }
    const std::size_t removed = mod->second.size();
    mMods.erase(mod);
    mBytesPerMod.erase(modId);
    MarkDirty();
    return Result<std::size_t>::ok(removed);
}

Result<std::vector<std::string>> KeyValueStorage::Keys(const std::string& modId) const {
    if (modId.empty()) {
        return Result<std::vector<std::string>>::err(ErrorCode::InvalidArgument,
                                                     "storage requires a mod id");
    }
    std::vector<std::string> keys;
    const auto mod = mMods.find(modId);
    if (mod != mMods.end()) {
        keys.reserve(mod->second.size());
        for (const auto& [key, value] : mod->second) {
            (void)value;
            keys.push_back(key);
        }
    }
    return Result<std::vector<std::string>>::ok(std::move(keys));
}

std::size_t KeyValueStorage::KeyCount(const std::string& modId) const {
    const auto mod = mMods.find(modId);
    return mod != mMods.end() ? mod->second.size() : 0;
}

std::size_t KeyValueStorage::TotalBytes(const std::string& modId) const {
    const auto entry = mBytesPerMod.find(modId);
    return entry != mBytesPerMod.end() ? entry->second : 0;
}

std::string KeyValueStorage::Serialize() const {
    std::string out;
    out.append(kMagic, sizeof(kMagic));
    out.push_back(static_cast<char>(kVersion));
    PutU32(out, static_cast<std::uint32_t>(mMods.size()));
    for (const auto& [modId, entries] : mMods) {
        PutBytes(out, modId);
        PutU32(out, static_cast<std::uint32_t>(entries.size()));
        for (const auto& [key, value] : entries) {
            PutBytes(out, key);
            std::visit(
                [&out](const auto& item) {
                    using T = std::decay_t<decltype(item)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        out.push_back(static_cast<char>(0));
                        out.push_back(static_cast<char>(item ? 1 : 0));
                    } else if constexpr (std::is_same_v<T, std::int64_t>) {
                        out.push_back(static_cast<char>(1));
                        PutU64(out, static_cast<std::uint64_t>(item));
                    } else if constexpr (std::is_same_v<T, double>) {
                        out.push_back(static_cast<char>(2));
                        std::uint64_t bits = 0;
                        std::memcpy(&bits, &item, sizeof(bits));
                        PutU64(out, bits);
                    } else { // std::string
                        out.push_back(static_cast<char>(3));
                        PutBytes(out, item);
                    }
                },
                value);
        }
    }
    return out;
}

Result<void> KeyValueStorage::Deserialize(std::string_view blob) {
    Reader reader{blob};
    const char* magic = nullptr;
    if (!reader.Take(sizeof(kMagic), magic) || std::memcmp(magic, kMagic, sizeof(kMagic)) != 0) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage blob has an invalid header");
    }
    if (reader.U8() != kVersion) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage blob has an unsupported version");
    }

    // Reconstrói num store temporário e só troca no fim: um blob corrompido no
    // meio não deixa o store atual pela metade.
    std::map<std::string, std::map<std::string, Value>> mods;
    std::map<std::string, std::size_t> bytesPerMod;

    const std::uint32_t modCount = reader.U32();
    for (std::uint32_t m = 0; m < modCount && reader.ok; ++m) {
        std::string modId = reader.Bytes();
        const std::uint32_t entryCount = reader.U32();
        for (std::uint32_t e = 0; e < entryCount && reader.ok; ++e) {
            std::string key = reader.Bytes();
            const std::uint8_t tag = reader.U8();
            Value value;
            switch (tag) {
                case 0:
                    value = reader.U8() != 0;
                    break;
                case 1:
                    value = static_cast<std::int64_t>(reader.U64());
                    break;
                case 2: {
                    const std::uint64_t bits = reader.U64();
                    double d = 0.0;
                    std::memcpy(&d, &bits, sizeof(d));
                    value = d;
                    break;
                }
                case 3:
                    value = reader.Bytes();
                    break;
                default:
                    reader.ok = false;
                    break;
            }
            if (!reader.ok || modId.empty() || key.empty()) {
                continue;
            }
            // Aplica os mesmos limites do Set: uma entrada fora do limite é
            // pulada, não derruba a carga inteira.
            const std::size_t entryBytes = key.size() + ValueBytes(value);
            if (key.size() > mLimits.maxKeyBytes || ValueBytes(value) > mLimits.maxValueBytes) {
                continue;
            }
            auto& entries = mods[modId];
            std::size_t& total = bytesPerMod[modId];
            if (entries.size() >= mLimits.maxKeysPerMod ||
                total + entryBytes > mLimits.maxTotalBytesPerMod) {
                continue;
            }
            total += entryBytes;
            entries[std::move(key)] = std::move(value);
        }
    }
    if (!reader.ok) {
        return Result<void>::err(ErrorCode::InvalidArgument, "storage blob is truncated or corrupt");
    }
    mMods = std::move(mods);
    mBytesPerMod = std::move(bytesPerMod);
    return Result<void>::ok();
}

void KeyValueStorage::MarkDirty() {
    if (mPersistPath.has_value()) {
        mDirty = true;
    }
}

bool KeyValueStorage::IsDirty() const {
    return mDirty;
}

void KeyValueStorage::SweepStaleTemporaries() const {
    if (!mPersistPath.has_value()) {
        return;
    }
    std::error_code ec;
    const std::filesystem::path parent =
        mPersistPath->parent_path().empty() ? std::filesystem::current_path(ec) : mPersistPath->parent_path();
    if (ec) {
        return;
    }
    const std::string prefix = mPersistPath->filename().string() + ".shiplua-tmp-";
    for (const auto& entry : std::filesystem::directory_iterator(parent, ec)) {
        if (ec) {
            return;
        }
        if (!entry.is_regular_file(ec) || ec) {
            ec.clear();
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (name.rfind(prefix, 0) == 0) {
            std::error_code ignored;
            std::filesystem::remove(entry.path(), ignored);
        }
    }
}

Result<void> KeyValueStorage::Flush() {
    if (!mDirty || !mPersistPath.has_value()) {
        return Result<void>::ok();
    }
    const auto written = AtomicFile::Write(*mPersistPath, Serialize());
    if (!written.isOk()) {
        // Mantém sujo para tentar de novo no próximo flush; um antivírus
        // segurando o arquivo é transitório.
        return written;
    }
    mDirty = false;
    return Result<void>::ok();
}

Result<void> KeyValueStorage::EnablePersistence(std::filesystem::path path) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        std::ifstream stream(path, std::ios::binary);
        if (stream) {
            std::string blob((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            const auto loaded = Deserialize(blob);
            if (!loaded.isOk()) {
                // Arquivo corrompido: não trava o host. Começa vazio e será
                // sobrescrito na próxima gravação. O chamador loga o motivo.
                mMods.clear();
                mBytesPerMod.clear();
                mPersistPath = std::move(path);
                SweepStaleTemporaries();
                return loaded;
            }
        }
    }
    mPersistPath = std::move(path);
    // Limpa temporários deixados por uma sessão anterior que morreu no meio de
    // uma gravação (ou cujo rename falhou por lock externo).
    SweepStaleTemporaries();
    return Result<void>::ok();
}

} // namespace ShipLua
