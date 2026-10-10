#!/usr/bin/env python3
"""Gera o header de tipos de áudio do MM isolado em namespace, para o host OoT.

Por que existe: os arquivos de áudio do Majora's Mask incluem global.h,
BenPort.h e headers do 2S2H, que arrastam o jogo inteiro. E o tipo
AudioContext tem o MESMO nome nos dois decomps, então o código portado
precisa viver num namespace — mas header de sistema não sobrevive dentro de
um. Copiar só as definições de tipo resolve os dois problemas de uma vez.

Uso:  py -3 tools/gen_mm_audio_types.py
Lê:   ../MM-MODSDK-001/mm/include/{audio/*.h,z64audio.h}
Grava: ../shipwright-limpo/Shipwright/soh/soh/mmaudio/mmseq/MmAudioTypes.h
"""
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INC = os.path.join(ROOT, '..', 'MM-MODSDK-001', 'mm', 'include')
OUT = os.path.join(ROOT, '..', 'shipwright-limpo', 'Shipwright', 'soh', 'soh',
                   'mmaudio', 'mmseq', 'MmAudioTypes.h')

# Ordem importa: tipos base antes de quem os usa.
#
# heap.h e load.h ENTRAM apesar de serem quase só declaração de função: eles
# definem 21 tipos que o AudioContext referencia (AudioSessionPoolSplit,
# AudioCachePoolSplit, SampleDma, AudioTable...). Sem eles o AudioContext não
# compila. As declarações de função que vêm junto são inofensivas — declarar
# não exige definir, e nada as chama; os 12 símbolos que o interpretador
# realmente usa viram shim próprio.
ORDER = [
    'audio/soundfont.h',
    'audio/heap.h',
    'audio/load.h',
    'audio/reverb.h',
    'audio/effects.h',
    'z64audio.h',
]

PREAMBLE = '''#pragma once

// Tipos de áudio do Majora's Mask, isolados em namespace próprio.
//
// GERADO por link-span/tools/gen_mm_audio_types.py — NÃO editar à mão.
//
// Colisões medidas contra soh/src/code/audio_*.c (handoff OOT-AUDIO-001):
//   seqplayer/playback/effects .... 0 funções
//   data.c ........................ 9 globais (tabelas)
//   synthesis.c ................... 19 funções AudioSynth_* (arquivo cortado)
//   contexto global ............... 0 (gAudioCtx no MM, gAudioContext no OoT)
//
// ============================ REGRA DURA ============================
// NÃO inclua este header na mesma unidade de tradução que o z64audio.h do
// OoT. O namespace isola os TIPOS, mas macro é preprocessador e não respeita
// namespace: IS_SEQUENCE_CHANNEL_VALID, NO_LAYER, SEQ_NUM_CHANNELS e
// TATUMS_PER_BEAT existem nos dois decomps e a redefinição vira erro.
//
// Os arquivos portados do MM incluem SÓ este header. A ponte com o resto do
// host é uma API C de tipos simples (int16_t, int), num header separado —
// mesmo desenho do mm_sfx_synth.h do fork skijer.
// ====================================================================

#include <cstddef>
#include <cstdint>

namespace mmsfx {

// Tipos do ultra64 redeclarados localmente: incluir PR/ultratypes.h do MM
// aqui traria as guardas do OoT junto e uma das duas definições perderia.
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef volatile u8 vu8;
typedef volatile s32 vs32;

// Ponteiro genérico do nó de lista encadeada do áudio. No decomp é um `void*`
// que recebe e devolve Note* ou SequenceLayer* livremente — legal em C, erro em
// C++. Este proxy converte nos dois sentidos, é trivialmente copiável (logo cabe
// numa union) e tem o mesmo tamanho de um ponteiro, preservando o layout.
struct AnyPtr {
    void* raw;

    // `= default` mantém o tipo trivialmente construtível, que é o que permite
    // usá-lo dentro de uma union sem apagar os membros especiais dela.
    AnyPtr() = default;
    AnyPtr(decltype(nullptr)) : raw(nullptr) {
    }
    template <typename T> AnyPtr(T* p) : raw(static_cast<void*>(p)) {
    }

    template <typename T> operator T*() const {
        return static_cast<T*>(raw);
    }
    template <typename T> AnyPtr& operator=(T* p) {
        raw = static_cast<void*>(p);
        return *this;
    }
    AnyPtr& operator=(decltype(nullptr)) {
        raw = nullptr;
        return *this;
    }
    bool operator==(const void* p) const {
        return raw == p;
    }
    bool operator!=(const void* p) const {
        return raw != p;
    }
    explicit operator bool() const {
        return raw != nullptr;
    }
};

// Macros de unk.h do MM. O decomp as usa em campos ainda não identificados;
// sem elas as structs de DMA não compilam.
#define UNK_TYPE s32
#define UNK_TYPE1 s8
#define UNK_TYPE2 s16
#define UNK_TYPE4 s32
#define UNK_TYPE8 s64
#define UNK_PTR void*
#define UNK_SIZE 1

// Tipos do SO do N64, como substitutos opacos. Só aparecem nas structs de DMA
// e de fila de mensagens (SampleDma, AudioAsyncLoad, AudioContext), que existem
// no header porque o AudioContext as embute por valor — mas o caminho de SFX
// não toca em nenhuma delas: quem carrega recurso aqui é o ResourceMgr do SoH.
// Os tamanhos não precisam bater com o N64; nada fora desta TU lê esses bytes.
struct OSMesgQueue {
    void* mtqueue;
    void* fullqueue;
    s32 validCount;
    s32 first;
    s32 msgCount;
    void** msg;
};
typedef void* OSMesg;
struct OSPiHandle {
    void* next;
    u8 type;
};
struct OSIoMesg {
    u32 hdr[2];
    void* dramAddr;
    u32 devAddr;
    u32 size;
    u8 piHandle;
};

// Tipos do RSP, também opacos. O AudioTask embute um OSTask por valor e as
// assinaturas de função customizada mencionam Acmd*, mas o caminho do RSP é
// justamente o que este porte corta: quem sintetiza PCM aqui é o decodificador
// da Fase 1, não o microcódigo emulado.
struct OSTask {
    u32 opaque[16]; // 0x40 bytes no N64
};
struct Acmd {
    u32 w0;
    u32 w1;
};
'''


def clean(path):
    """Remove includes e a guarda de header, preservando #if/#endif legítimos.

    A guarda tem que sair CASADA: tirar o `#ifndef X` / `#define X` sem tirar o
    `#endif` do fim deixa um `#endif` órfão, e o compilador só reclama depois de
    concatenar tudo — longe da causa.
    """
    with open(os.path.join(INC, path), encoding='utf-8', errors='replace') as fh:
        lines = fh.read().split('\n')

    drop = set()

    # Guarda clássica: #ifndef X seguido de #define X nas primeiras linhas.
    for i, line in enumerate(lines[:20]):
        match = re.match(r'#ifndef\s+(\w+)', line.strip())
        if not match:
            continue
        nxt = lines[i + 1].strip() if i + 1 < len(lines) else ''
        if re.match(r'#define\s+%s\b' % re.escape(match.group(1)), nxt):
            drop.add(i)
            drop.add(i + 1)
            # o #endif da guarda é o último do arquivo
            for j in range(len(lines) - 1, -1, -1):
                if lines[j].strip().startswith('#endif'):
                    drop.add(j)
                    break
            break

    # heap.h e load.h entram só pelos TIPOS. As declarações de função que vêm
    # junto colidem com os nossos shims — o AudioHeap_SearchCaches de lá é
    # `void*`, o nosso devolve AutoPtr, e o compilador vê sobrecarga que difere
    # só no retorno.
    strip_protos = path in ('audio/heap.h', 'audio/load.h')
    proto = re.compile(r'^[A-Za-z_][A-Za-z0-9_ \*]*\b[A-Za-z_][A-Za-z0-9_]*\s*\([^;{]*\)\s*;')

    out = []
    for i, line in enumerate(lines):
        if i in drop:
            continue
        stripped = line.strip()
        # includes saem: os tipos base são declarados no preâmbulo, e header de
        # sistema não pode entrar dentro de namespace.
        if stripped.startswith('#include'):
            continue
        if stripped.startswith('#pragma once'):
            continue
        if strip_protos and proto.match(stripped):
            continue
        out.append(line)
    return '\n'.join(out)


def main():
    parts = [PREAMBLE]
    for path in ORDER:
        parts.append('\n// ===================== %s =====================\n' % path)
        parts.append(clean(path))
    parts.append('\n} // namespace mmsfx\n')

    body = '\n'.join(parts)

    # O `void* value` do nó de lista vira AnyPtr: é o único campo do decomp que
    # recebe e devolve ponteiros de tipos diferentes, e em C++ isso não compila.
    body = body.replace('/* 0x08 */ void* value;', '/* 0x08 */ AnyPtr value;')
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(body)
    print('gerado: %s (%d linhas)' % (os.path.normpath(OUT), body.count('\n')))


if __name__ == '__main__':
    main()
