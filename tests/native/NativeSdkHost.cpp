#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "shiplua/host/ModHost.h"
#include "shiplua/manifest/PackageFormat.h"
#include "../../examples/native-dash/player_service.h"

namespace {
double SHIP_NATIVE_CALL GetVelocity(void* context) { return *static_cast<double*>(context); }
void SHIP_NATIVE_CALL SetVelocity(void* context, double value) { *static_cast<double*>(context) = value; }
}

// Host de demonstração ROM-free: recompilar SÓ o provider deve mudar o gameplay
// simulado. Não é evidência de execução dentro de Shipwright/2Ship.
int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Uso: native_sdk_host <pasta|pacote.zip|pacote.shipmod> <velocidade-esperada> [oot|mm]\n";
        return 2;
    }
    double velocity = 2.0;
    ExamplePlayerService player{&velocity, GetVelocity, SetVelocity};
    ShipLua::NativeProviderPolicy native;
    native.enabled = true; // Este executável é um harness explícito de mods nativos.
    native.services = {{"example.player", 1, sizeof(player), &player}};
    ShipLua::LuaApiHostContext context;
    context.gameId = argc == 4 ? argv[3] : "oot";
    context.hostVersion = "1.0.0";
    ShipLua::ModHost host(context, {}, native);
    const std::filesystem::path path(argv[1]);
    auto result = ShipLua::HasModPackageExtension(path)
        ? host.LoadModFromPackage(path.string(), (path.parent_path()/"native-cache").string())
        : host.LoadModFromDirectory(path.string());
    if (!result.isOk()) { std::cerr << result.message << '\n'; return 1; }
    ShipLua::EventPayload payload;
    auto event = host.DispatchEvent("game.ready", payload);
    const double expected = std::strtod(argv[2], nullptr);
    if (!event.isOk() || !std::isfinite(expected) || std::abs(velocity-expected) > 0.001) {
        std::cerr << "Dash não alterou velocidade: " << velocity << '\n'; return 1;
    }
    std::cout << "Dash definido pelo mod: velocidade " << velocity << '\n';
    for (int frame=1; frame<=3; ++frame) {
        payload["frame"] = static_cast<std::int64_t>(frame);
        event = host.DispatchEvent("game.frame", payload);
        if (!event.isOk()) return 1;
    }
    if (velocity != 2.0) { std::cerr << "Velocidade não foi restaurada\n"; return 1; }
    host.DispatchEvent("game.ready", payload);
    result = host.UnloadMod("example.native_dash");
    if (!result.isOk() || velocity != 2.0 || host.SubscriptionCount() != 0) return 1;
    std::cout << "Gameplay simulado, eventos e cleanup aprovados em " << context.gameId << ".\n";
    return 0;
}
