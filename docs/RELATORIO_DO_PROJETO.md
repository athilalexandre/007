# Relatório Completo de Desenvolvimento — GoldenEye 007 (N64 Decompilation to WebAssembly/WebGL)

**Data de Atualização:** 24 de Agosto de 2026  
**Repositório:** `007`  
**Branch Atual:** `feat/playable-single-player-v1`  
**Base:** Decompilação autêntica em C do clássico *GoldenEye 007* (Nintendo 64, Rareware, 1997) portado nativamente para WebAssembly (WASM), WebGL 2.0 e WebSockets Netplay.

---

## 1. Visão Geral Executiva

O objetivo principal deste projeto é transformar a decompilação de código aberto do jogo **GoldenEye 007 (N64)** em uma versão moderna, 100% nativa para navegadores web, preservando com exatidão matemática o comportamento original da engine (física, IA de guardas, scripts de missão, matemática de tiro e animação), combinando com:
1. **Compilação C para WebAssembly (WASM)** com Emscripten/Clang de alta performance.
2. **Camada de Abstração de Hardware (Web HAL)** que virtualiza o hardware N64 (CPU MIPS, Coprocessadores, RCP/Fast3D, DMA do Cartucho PI, e Controladores).
3. **Pipeline Gráfico WebGL 2.0** com rasterização autêntica de display lists Fast3D (GBI), texturização e pós-processamento de tela.
4. **Interface Moderna (Glassmorphism Web UI)** com suporte a teclado/mouse (WASD + Mouse Look), Gamepads e controles configuráveis.
5. **Netplay Multiplayer Autoritativo** via WebSockets com suporte a Modo Cooperativo (2 Jogadores) e Multiplayer Competitivo (até 16 Jogadores).

---

## 2. Linha do Tempo e Marcos Concluídos

### Marco 1: Recuperação e Compilação Inicial em WebAssembly (`3cd052d0`)
- **Criação do `Makefile.web`**: Configuração completa do pipeline Emscripten para compilar dezenas de arquivos fonte C originais em um único módulo WebAssembly (`goldeneye007.wasm` e `goldeneye007.js`).
- **Implementação do Web HAL (`src/platform/web/`)**:
  - `hal_os.c` / `hal_os.h`: Emulação do ambiente operacional N64 (Ultra64 OS), gerenciamento de threads cooperativas, filas de mensagens e virtualização de memória.
  - `hal_gfx.c` / `hal_gfx.h`: Interpretador básico de comandos GBI / F3DEX para capturar comandos de renderização da engine.
  - `hal_input.c` / `hal_input.h`: Ponte de entrada convertendo eventos web de teclado, mouse e gamepad no formato de registro `OSContPad` do N64.
  - `rom_resolver.c` / `rom_resolver.h`: Mapeador estático de recursos apontando offsets de arquivos diretamente para a ROM autêntica (*GoldenEye 007 USA Retail - SHA-1 `abe01e4aeb033b6c0836819f549c791b26cfde83`*).
  - Substituição de rotinas Assembly MIPS específicas por equivalentes portáteis em C (`math_sincos_c.c`, `random_c.c`).
- **Testes Automatizados**: Implementação de suites de teste Node.js (`test_integrity.js` e `test_engine.js`).

### Marco 2: Prova de Vida da Engine e Heap Autêntico (`7c91898d`)
- **Descompressão e DMA Real**: Integração do algoritmo de descompressão in-engine (`src/inflate/inflate.c` e `src/game/decompress.c`) executando diretamente sobre a memória heap permanente virtualizada.
- **Correção de Corrupção de Memória**:
  - Ajuste no dimensionamento do pool de colisões `ModelHitEntry` em `objecthandler.c` e `initunk_005450.c`, eliminando estouros de pilha/BSS em 32-bit WASM.
  - Alinhamento de tabelas de símbolos e fontes em `front.c`, `textrelated.c` e `lv.c`.
- **Renderização da Tela Legal / Título**: Execução estável do loop de inicialização e exibição das telas de introdução da engine.

### Marco 3: WebGL 2.0 Fast3D, HUD Autêntico e Netplay Multiplayer (`aa6ac7cc`)
- **Renderizador GPU WebGL 2.0 (`web/gl_renderer.js`)**:
  - Shaders customizados GLSL (Vertex + Fragment) com blend de framebuffer e efeitos de flash de dano.
  - HUD autêntico renderizado via Canvas 2D composto sobre a tela 3D: Barras de Vida (Vermelha) e Colete (Azul), mira dinâmica responsiva ao recuo, contadores de munição e identificação de armas.
- **Arquitetura de Netplay (`server.js` + `web/netplay.js`)**:
  - Servidor WebSocket autoritativo com suporte a salas privadas/públicas.
  - Modo Cooperativo para 2 Jogadores na Campanha.
  - Modo Deathmatch / Team Deathmatch para até 16 jogadores com feed de eliminações, sincronização de posições e placar ao vivo.
- **Interface e Seletor de Fases (`web/index.html` + `web/app.js`)**:
  - Todas as 20 missões da campanha (Dam até Egyptian) e 6 arenas multiplayer.
  - Seletor de personagens (Bond, Trevelyan, Natalya, Boris, Oddjob, Jaws).

### Marco 4: Ajuste do Rasterizador, Orientação de Viewport e Console Diagnóstico (`992af570`)
- **Correção de Mapeamento UV / Viewport**: Ajuste das coordenadas verticais e mapeamento de texturas para eliminar inversões de tela.
- **Console Diagnóstico em Tempo Real**:
  - Sistema de telemetria e logs integrados na UI com filtros por categoria (`SYS`, `ROM`, `GFX`, `NET`, `INPUT`, `LVL`, `PERF`).
  - Monitoramento de comandos GBI, contagem de vértices, triângulos, FPS e tempo de frame.

---

## 3. O Que Estávamos Fazendo (Fase Atual em `feat/playable-single-player-v1`)

Atualmente, o foco ativo de desenvolvimento está na **estabilização da jogabilidade da Campanha Single-Player (Missão 1: Dam)** e transição completa de fases com IA ativa.

### Atividades em Andamento:
1. **Validação do Ciclo de Carregamento de Fase (`src/game/lv.c`)**:
   - Verificação da sequência de inicialização: `load_bg_file()`, `init_watch_at_start_of_stage()`, `init_guards()`, `init_player_BONDdata_stats()`, etc.
   - Confirmação de que o sistema de guardas carrega os inimigos da represa com sucesso (10 guardas carregados no teste `test_gameplay.js`).
   - Spawn correto do James Bond na posição `[0.00, 2512.66, 0.00]` com inventário inicial (PP7 Special Issue).

2. **Resolução de Chamadas DMA Fora dos Limites (`devAddr=0x77777776`)**:
   - Investigação de acessos residuais a ponteiros de dados brutos de modelos/texturas durante a descompressão de cenários complexos no arquivo `src/game/model.c` e `src/game/bg.c`.
   - Garantir que todos os ponteiros de texturas passem pelo `rom_resolver` ou pelos bancos de imagem em RAM sem disparar fallbacks de endereço inválido.

3. **Profundidade de Cor e Rasterização do Framebuffer**:
   - Ajustar o fluxo de comandos de preenchimento (`G_SETFILLCOLOR`, `G_FILLRECT`, `G_TRI_1`) no rasterizador `hal_gfx.c` para que a paleta de cores completa da geometria 3D seja gravada no buffer RGBA5551 e exibida no WebGL.

---

## 4. Arquitetura do Sistema

```
+-------------------------------------------------------------------------+
|                              Navegador Web                              |
|                                                                         |
|  +---------------------------+       +-------------------------------+  |
|  |     Interface Web         |       |      WebGL 2.0 Fast3D         |  |
|  |  (index.html / app.js)    | <---> |   (gl_renderer.js / Canvas)   |  |
|  | - Seletor de Missões/Pers |       | - Framebuffer Blit            |  |
|  | - Painel Diagnóstico      |       | - HUD Dinâmico 007 (2D)       |  |
|  | - Controles / Sensib.     |       | - Shaders de Efeitos          |  |
|  +---------------------------+       +-------------------------------+  |
|               ^                                      ^                  |
|               |                                      |                  |
|               v                                      v                  |
|  +-------------------------------------------------------------------+  |
|  |                 Camada Web HAL (C / WebAssembly)                  |  |
|  |  - hal_os.c (Memória Heap, Threads, Timing)                       |  |
|  |  - hal_gfx.c (Interpretador GBI / F3DEX, Rasterizador)            |  |
|  |  - hal_input.c (Teclado WASD, Mouse Look, Gamepad API)            |  |
|  |  - rom_resolver.c (Descompressão e Mapeamento de Recursos)        |  |
|  +-------------------------------------------------------------------+  |
|                                  ^                                      |
|                                  v                                      |
|  +-------------------------------------------------------------------+  |
|  |            Engine Original C (GoldenEye 007 Decompilation)        |  |
|  |  - lv.c / bg.c / model.c (Carregamento de Fases, Cenários e Props)|  |
|  |  - chrai.c / chr.c (Inteligência Artificial de Guardas e Chefes)  |  |
|  |  - bondinv.c / gun.c / gunfire.c (Armas, Munição, Tiro, Dano)    |  |
|  |  - player.c / bondview.c (Câmera, Movimentação e Física do Bond)  |  |
|  +-------------------------------------------------------------------+  |
+-------------------------------------------------------------------------+
                                   ^
                                   | WebSockets (JSON / Binary)
                                   v
+-------------------------------------------------------------------------+
|                  Servidor Node.js (server.js)                           |
|  - Matchmaking e Salas (2P Co-op & até 16P Multiplayer)                 |
|  - Sincronização Autoritativa de Estados e Posições                     |
|  - Servidor de Arquivos Estáticos HTTP/WASM                             |
+-------------------------------------------------------------------------+
```

---

## 5. Estrutura de Arquivos Principais

| Caminho | Descrição |
|---|---|
| `Makefile.web` | Script de compilação Emscripten para gerar `goldeneye007.wasm` e `goldeneye007.js`. |
| `server.js` | Servidor Node.js com WebSockets para lobby, salas multiplayer e servidor estático. |
| `src/platform/web/hal_os.c` | Virtualização de hardware do N64 (memória, DMA, filas de mensagens). |
| `src/platform/web/hal_gfx.c` | Interpretador de microcódigo GBI / Fast3D e rasterizador de framebuffers. |
| `src/platform/web/hal_input.c` | Mapeamento de entradas do navegador (teclado, mouse, gamepad). |
| `src/platform/web/rom_resolver.c` | Resolução de arquivos da ROM americana do GoldenEye 007. |
| `src/game/lv.c` | Gerenciador mestre de carregamento de fases, spawn de jogadores e objetivos. |
| `src/game/model.c` | Parser de modelos 3D de personagens, armas e objetos interativos. |
| `web/index.html` | Interface moderna de usuário, tela de missões, lobby e diagnóstico. |
| `web/app.js` | Orquestrador principal da aplicação web, controle de ciclo de vida e inputs. |
| `web/gl_renderer.js` | Renderizador WebGL 2.0 com suporte a HUD, pós-processamento e telemetria. |
| `web/netplay.js` | Cliente WebSocket para gerenciamento de partidas cooperativas e multiplayer. |
| `tests/test_gameplay.js` | Teste automatizado validando a inicialização e jogabilidade da missão Dam. |

---

## 6. Próximos Passos Imediatos

1. **Eliminar Acessos DMA Inválidos**:
   - Tratar ponteiros de recursos não indexados na rotina de extração de modelos e texturas de nível.
2. **Expandir Rasterização de Polígonos 3D no WebGL**:
   - Alimentar os buffers de vértices e triângulos do WebGL diretamente através das display lists GBI geradas pela engine.
3. **Mapeamento de Áudio Completo (Web Audio API)**:
   - Conectar os canais de áudio da engine (`snd.c`, `music.c`) para síntese de efeitos sonoros e trilhas sonoras orquestradas autênticas de Graeme Norgate e Grant Kirkhope.
4. **Validação de Todas as Missões**:
   - Automatizar testes de regressão passando sequencialmente pelas 20 missões da campanha.
