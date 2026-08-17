/**
 * GoldenEye 007 Web Port - Main Entry Point
 * This replaces the N64 boot sequence (_start.s -> boot.s -> init.c -> boss.c)
 * with a web-friendly initialization that uses Emscripten main loop.
 */
#ifdef TARGET_WEB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

/* Forward declarations from the original game code */
extern void bossEntry(void);
extern void osInitialize(void);

/* HAL functions */
extern void hal_gfx_init(int width, int height);
extern void hal_os_set_rom_data(unsigned char *data, unsigned int size);

static int g_gameInitialized = 0;
static int g_romLoaded = 0;

/**
 * Main loop callback for Emscripten.
 * Called once per frame by emscripten_set_main_loop.
 */
static void web_main_loop(void) {
    if (!g_gameInitialized || !g_romLoaded) {
        /* Waiting for ROM to be loaded from JavaScript side */
        return;
    }

    /* The original game loop is driven by bossEntry() which contains
     * the main game loop. On N64 it runs forever in a thread.
     * For web, we need to make it yield control each frame.
     * This requires modifying the game loop to be non-blocking.
     *
     * TODO: Restructure bossMainloop() to process one frame per call
     * instead of looping forever.
     */
}

/**
 * Called from JavaScript when the user provides their ROM file.
 * This triggers the actual game initialization.
 */
void EMSCRIPTEN_KEEPALIVE web_load_rom(unsigned char *data, unsigned int size) {
    printf("[WEB] ROM loaded: %u bytes\n", size);

    /* Set ROM data in HAL layer */
    hal_os_set_rom_data(data, size);
    g_romLoaded = 1;

    /* Initialize the game */
    printf("[WEB] Initializing game engine...\n");

    /* On N64: init() -> osInitialize() -> mainproc() -> bossEntry()
     * On web: we call these directly since there are no real threads */
    osInitialize();

    /* Initialize WebGL renderer */
    hal_gfx_init(960, 720);

    g_gameInitialized = 1;
    printf("[WEB] Game initialized! Starting main loop.\n");
}

/**
 * Main entry point for the web build.
 * On N64, the boot sequence is:
 *   _start.s -> boot.s -> init() -> mainproc() -> bossEntry()
 *
 * On web, we:
 *   1. Start the Emscripten main loop
 *   2. Wait for the user to provide a ROM file via the UI
 *   3. Initialize the game when ROM is loaded
 *   4. Run the game loop via emscripten_set_main_loop
 */
int main(int argc, char *argv[]) {
    printf("===========================================\n");
    printf("  GoldenEye 007 - Web Browser Port\n");
    printf("  Based on n64decomp/007 decompilation\n");
    printf("===========================================\n");
    printf("[WEB] Waiting for ROM file...\n");
    printf("[WEB] Please use the file input to load your GoldenEye 007 .z64 ROM.\n");

#ifdef __EMSCRIPTEN__
    /* Set up the main loop at 60 FPS */
    emscripten_set_main_loop(web_main_loop, 60, 0);
#else
    /* Non-Emscripten fallback for testing */
    printf("[WEB] Running without Emscripten - exiting.\n");
#endif

    return 0;
}

#endif /* TARGET_WEB */