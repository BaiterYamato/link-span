#!/usr/bin/env python3
"""Porta um arquivo de áudio do decomp de MM para dentro do host OoT.

O que faz, e só isso — a transformação é deliberadamente mecânica para que o
resultado continue diffável contra o decomp original:

  1. troca os #include do MM (global.h, BenPort.h, headers do 2S2H) pelo nosso
     MmAudioContext.h, que traz os tipos em namespace e os shims;
  2. envolve o corpo em `namespace mmsfx { ... }`;
  3. marca o cabeçalho dizendo de onde veio.

Por que os includes originais não servem: global.h arrasta o jogo inteiro, e o
tipo AudioContext tem o mesmo nome nos dois decomps. E por que namespace não
basta sozinho: macro é preprocessador e não respeita namespace — daí a regra de
que arquivo portado nunca inclui header do OoT (ver OOT-AUDIO-001 Fase 2).

Uso:  py -3 tools/port_mm_audio_file.py effects.c
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, '..', 'MM-MODSDK-001', 'mm', 'src', 'audio', 'lib')
DST = os.path.join(ROOT, '..', 'shipwright-limpo', 'Shipwright', 'soh', 'soh',
                   'mmaudio', 'mmseq')

HEADER = '''// PORTADO de MM-MODSDK-001/mm/src/audio/lib/%s por
// link-span/tools/port_mm_audio_file.py — regenerável.
//
// A única transformação é: includes trocados e corpo dentro de namespace mmsfx.
// O resto é o decomp literal, para continuar diffável contra o upstream.
//
// Ver coordination/handoffs/OOT-AUDIO-001-port-mm-sfx.md.

#include "soh/mmaudio/mmseq/MmAudioContext.h"
#include "soh/mmaudio/mmseq/MmAudioDecls.h"

#ifdef _MSC_VER
// Conversões implícitas com perda são deliberadas no decomp (o hardware do N64
// trunca do mesmo jeito) e não são nossas para consertar — mexer nelas quebraria
// o diff contra o upstream. Silenciadas SÓ neste arquivo; o resto do host segue
// compilando com aviso-como-erro.
#pragma warning(push)
#pragma warning(disable : 4244) // conversão com possível perda de dados
#pragma warning(disable : 4245) // conversão signed/unsigned
#pragma warning(disable : 4305) // truncamento em inicialização
#pragma warning(disable : 4267) // size_t para tipo menor
#pragma warning(disable : 4101) // variável local não referenciada (os `pad` do decomp)
#pragma warning(disable : 4805) // mistura de bool com inteiro em comparação
#endif

namespace mmsfx {

'''

FOOTER = '''
} // namespace mmsfx

#ifdef _MSC_VER
#pragma warning(pop)
#endif
'''


# Transformações textuais pontuais, mantidas curtas e explícitas de propósito —
# cada uma aqui é uma linha a menos de diff limpo contra o decomp, então só entra
# o que não dá para resolver no header.
#
# AudioScript_AudioListPopBack é a ÚNICA função dos quatro arquivos portados que
# devolve `void*` (as outras estão em heap.c/load.c, que não portamos). Em C o
# retorno vira Note* ou SequenceLayer* sozinho; em C++ não. Trocar pelo proxy
# AnyPtr resolve nos dois sentidos sem tocar nos chamadores.
TRANSFORMS = [
    ('void* AudioScript_AudioListPopBack(', 'AnyPtr AudioScript_AudioListPopBack('),

    # As sete restantes são todas o mesmo caso: atribuir um `void*` (parâmetro
    # ou cast explícito do decomp) a um ponteiro tipado. C aceita, C++ não.
    # Anotar o tipo de destino é a mudança mínima — não muda semântica nenhuma,
    # só torna explícito o que o compilador C já fazia sozinho.
    ('channel->scriptState.pc = script;',
     'channel->scriptState.pc = (u8*)script;'),
    ('gAudioCtx.soundFontList[fontId].drums[index] = value;',
     'gAudioCtx.soundFontList[fontId].drums[index] = (Drum*)value;'),
    ('gAudioCtx.soundFontList[fontId].instruments[index] = value;',
     'gAudioCtx.soundFontList[fontId].instruments[index] = (Instrument*)value;'),
    # Sem o índice no padrão: o decomp usa cmdArgU16 num ponto e
    # channel->unk_22 no outro, e casar a linha inteira deixaria o segundo
    # passar batido — foi o que aconteceu na primeira tentativa.
    ('channel->dynTable = (void*)&seqPlayer->seqData[',
     'channel->dynTable = (u8(*)[][2]) & seqPlayer->seqData['),
    ('scriptState->pc = (void*)&seqPlayer->seqData[cmdArgU16];',
     'scriptState->pc = (u8*)&seqPlayer->seqData[cmdArgU16];'),
]


def port(name):
    with open(os.path.join(SRC, name), encoding='utf-8', errors='replace') as fh:
        lines = fh.read().split('\n')

    body = [line for line in lines if not line.strip().startswith('#include')]

    text = '\n'.join(body)
    for old, new in TRANSFORMS:
        text = text.replace(old, new)

    out = HEADER % name + text + FOOTER
    dst = os.path.join(DST, name.replace('.c', '.cpp'))
    os.makedirs(DST, exist_ok=True)
    with open(dst, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(out)
    print('portado: %s -> %s (%d linhas)' % (name, os.path.basename(dst), out.count('\n')))


DECLS_HEADER = '''#pragma once

// Declarações cruzadas entre os arquivos de áudio portados do MM.
//
// GERADO por link-span/tools/port_mm_audio_file.py — NÃO editar à mão.
//
// Existe porque os arquivos do decomp se chamam entre si sem header comum: no
// MM as declarações vinham de global.h, que aqui não pode entrar. Extrair as
// assinaturas em vez de listá-las à mão evita que a lista envelheça em silêncio
// quando um arquivo for reportado.

#include "soh/mmaudio/mmseq/MmAudioTypes.h"

namespace mmsfx {

'''


def scan_decls(names):
    """Extrai assinaturas de função e globais dos arquivos portados."""
    import re as _re
    funcs, globs = [], []
    for name in names:
        path = os.path.join(DST, name.replace('.c', '.cpp'))
        with open(path, encoding='utf-8', errors='replace') as fh:
            src = fh.read()
        # definição de função no escopo de arquivo: `tipo nome(args) {`.
        # O separador aceita `*` colado no tipo (`TunedSample* Foo(...)`), senão
        # toda função que devolve ponteiro escapa — foi o que aconteceu com as
        # AudioPlayback_Get* na primeira versão.
        for m in _re.finditer(
                r'^((?:[A-Za-z_][A-Za-z0-9_]*[\s\*]+)+?)([A-Za-z_][A-Za-z0-9_]*)\s*\(([^;{]*)\)\s*\{',
                src, _re.M):
            ret, fname, args = m.group(1).strip(), m.group(2), m.group(3).strip()
            if ret.startswith('static') or fname in ('if', 'for', 'while', 'switch'):
                continue
            funcs.append('%s %s(%s);' % (ret, fname, args or 'void'))
        # global no escopo de arquivo: `tipo nome[] = ` ou `tipo nome = `
        for m in _re.finditer(
                r'^((?:[A-Za-z_][A-Za-z0-9_]*\s+)+\**)([A-Za-z_][A-Za-z0-9_]*)((?:\[[^\]]*\])*)\s*=',
                src, _re.M):
            typ, gname, dims = m.group(1).strip(), m.group(2), m.group(3)
            if typ.startswith('static') or not gname[0] in 'gs':
                continue
            # dimensões viram [] na declaração extern
            globs.append('extern %s %s%s;' % (typ, gname, _re.sub(r'\[[^\]]*\]', '[]', dims)))
    return sorted(set(funcs)), sorted(set(globs))


def write_decls(names):
    funcs, globs = scan_decls(names)
    body = DECLS_HEADER
    body += '// ---- globais ----\n' + '\n'.join(globs) + '\n\n'
    body += '// ---- funções ----\n' + '\n'.join(funcs) + '\n'
    body += '\n} // namespace mmsfx\n'
    dst = os.path.join(DST, 'MmAudioDecls.h')
    with open(dst, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(body)
    print('declarações: MmAudioDecls.h (%d funções, %d globais)' % (len(funcs), len(globs)))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    for arg in sys.argv[1:]:
        port(arg)
    write_decls(sys.argv[1:])
