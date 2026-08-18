const ModuleFactory = require('../build/web/goldeneye007.js');

async function test() {
    console.log("Loading GoldenEye 007 WASM Module...");
    const Module = await ModuleFactory();
    console.log("Module initialized successfully.");

    // Check exported public functions
    console.log("Exported functions:");
    console.log("  _hal_engine_init:", typeof Module._hal_engine_init);
    console.log("  _hal_engine_is_initialized:", typeof Module._hal_engine_is_initialized);
    console.log("  _hal_os_set_rom_data:", typeof Module._hal_os_set_rom_data);
    console.log("  _hal_os_get_rom_status:", typeof Module._hal_os_get_rom_status);
    console.log("  _hal_get_build_info:", typeof Module._hal_get_build_info);

    // Initial state check
    const isInitBefore = Module._hal_engine_is_initialized();
    console.log("Engine initialized before hal_engine_init:", isInitBefore);

    // Call hal_engine_init()
    console.log("Calling _hal_engine_init()...");
    const initResult = Module._hal_engine_init();
    console.log("Result of _hal_engine_init():", initResult);

    const isInitAfter = Module._hal_engine_is_initialized();
    console.log("Engine initialized after hal_engine_init:", isInitAfter);

    if (initResult === 1 && isInitAfter === 1) {
        console.log("\n>>> SUCCESS: GoldenEye 007 engine initialized and executed cleanly via bossInitMainthreadData! <<<");
    } else {
        console.error(">>> FAILURE: Initialization did not return success <<<");
        process.exit(1);
    }
}

test().catch(err => {
    console.error("Error running test:", err);
    process.exit(1);
});