/**
 * GoldenEye 007 Web — Modern Keyboard & Mouse FPS Controls + Gamepad Mapper
 */

class AgentControls {
    constructor(canvas, onFireCallback, onActionCallback) {
        this.canvas = canvas;
        this.onFireCallback = onFireCallback;
        this.onActionCallback = onActionCallback;

        // Controller Bitmasks (Standard N64 libultra constants)
        this.CONT_A      = 0x8000;
        this.CONT_B      = 0x4000;
        this.CONT_Z      = 0x2000;
        this.CONT_START  = 0x1000;
        this.CONT_UP     = 0x0800;
        this.CONT_DOWN   = 0x0400;
        this.CONT_LEFT   = 0x0200;
        this.CONT_RIGHT  = 0x0100;
        this.CONT_L      = 0x0020;
        this.CONT_R      = 0x0010;
        this.CONT_E      = 0x0008;
        this.CONT_D      = 0x0004;
        this.CONT_C      = 0x0002;
        this.CONT_F      = 0x0001;

        // C-Buttons
        this.CONT_C_UP   = 0x0008;
        this.CONT_C_DOWN = 0x0004;
        this.CONT_C_LEFT = 0x0002;
        this.CONT_C_RIGHT= 0x0001;

        // Input state
        this.keys = {};
        this.mouseButtons = {};
        this.mouseDeltaX = 0;
        this.mouseDeltaY = 0;
        this.sensitivity = 1.0;
        this.isPointerLocked = false;

        // Current pad state
        this.buttons = 0;
        this.stickX = 0;
        this.stickY = 0;

        this.initListeners();
    }

    initListeners() {
        // Pointer Lock on click
        this.canvas.addEventListener('click', () => {
            if (!this.isPointerLocked) {
                this.canvas.requestPointerLock();
            }
        });

        document.addEventListener('pointerlockchange', () => {
            this.isPointerLocked = (document.pointerLockElement === this.canvas);
            const hint = document.getElementById('mouse-lock-hint');
            if (hint) {
                hint.style.display = this.isPointerLocked ? 'none' : 'block';
            }
        });

        // Mouse Move for Camera Aim
        document.addEventListener('mousemove', (e) => {
            if (this.isPointerLocked) {
                this.mouseDeltaX += e.movementX * this.sensitivity;
                this.mouseDeltaY += e.movementY * this.sensitivity;
            }
        });

        // Mouse Click
        document.addEventListener('mousedown', (e) => {
            if (!this.isPointerLocked) return;
            this.mouseButtons[e.button] = true;
            if (e.button === 0 && this.onFireCallback) {
                this.onFireCallback();
            }
        });

        document.addEventListener('mouseup', (e) => {
            this.mouseButtons[e.button] = false;
        });

        // Keyboard inputs
        window.addEventListener('keydown', (e) => {
            this.keys[e.code] = true;
            // Prevent scrolling on space / arrow keys
            if (['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Tab'].includes(e.code)) {
                e.preventDefault();
            }
            if (e.code === 'KeyE' && this.onActionCallback) {
                this.onActionCallback();
            }
        });

        window.addEventListener('keyup', (e) => {
            this.keys[e.code] = false;
        });

        // Wheel for weapon cycle
        window.addEventListener('wheel', (e) => {
            if (this.isPointerLocked) {
                if (e.deltaY > 0) {
                    this.triggerWeaponCycleNext();
                } else if (e.deltaY < 0) {
                    this.triggerWeaponCyclePrev();
                }
            }
        });
    }

    triggerWeaponCycleNext() {
        this.buttons |= this.CONT_A;
        setTimeout(() => { this.buttons &= ~this.CONT_A; }, 120);
    }

    triggerWeaponCyclePrev() {
        this.buttons |= this.CONT_B;
        setTimeout(() => { this.buttons &= ~this.CONT_B; }, 120);
    }

    poll() {
        let buttons = 0;
        let stickX = 0;
        let stickY = 0;

        // Keyboard Movement (WASD)
        const moveForward  = this.keys['KeyW'] || this.keys['ArrowUp'];
        const moveBackward = this.keys['KeyS'] || this.keys['ArrowDown'];
        const strafeLeft   = this.keys['KeyA'] || this.keys['ArrowLeft'];
        const strafeRight  = this.keys['KeyD'] || this.keys['ArrowRight'];

        if (moveForward)  stickY += 80;
        if (moveBackward) stickY -= 80;
        if (strafeLeft)   stickX -= 80;
        if (strafeRight)  stickX += 80;

        // Normalize diagonal speed
        if (stickX !== 0 && stickY !== 0) {
            stickX = Math.round(stickX * 0.7071);
            stickY = Math.round(stickY * 0.7071);
        }

        // Mouse Aim mapping to look (C-Buttons / Analog Aim)
        if (this.mouseDeltaX !== 0) {
            const turnFactor = Math.max(-80, Math.min(80, Math.round(this.mouseDeltaX * 12)));
            stickX = turnFactor;
            this.mouseDeltaX *= 0.4;
            if (Math.abs(this.mouseDeltaX) < 0.1) this.mouseDeltaX = 0;
        }

        if (this.mouseDeltaY !== 0) {
            // Pitch look
            if (this.mouseDeltaY > 1) buttons |= this.CONT_C_DOWN;
            if (this.mouseDeltaY < -1) buttons |= this.CONT_C_UP;
            this.mouseDeltaY *= 0.4;
            if (Math.abs(this.mouseDeltaY) < 0.1) this.mouseDeltaY = 0;
        }

        // Fire (Left Click) -> Z Trigger
        if (this.mouseButtons[0] || this.keys['KeyF']) {
            buttons |= this.CONT_Z;
        }

        // Aim Down Sights (Right Click) -> R Trigger
        if (this.mouseButtons[2]) {
            buttons |= this.CONT_R;
        }

        // Action / Open Doors (E / Space) -> B Button / Action
        if (this.keys['KeyE'] || this.keys['Space']) {
            buttons |= this.CONT_B;
        }

        // Reload (R) -> B Button 2x / Down-C
        if (this.keys['KeyR']) {
            buttons |= this.CONT_B | this.CONT_C_DOWN;
        }

        // Crouch (C / Control) -> Down-C / L Button
        if (this.keys['KeyC'] || this.keys['ControlLeft']) {
            buttons |= this.CONT_L | this.CONT_C_DOWN;
        }

        // Weapon Switch (Q) -> A Button
        if (this.keys['KeyQ']) {
            buttons |= this.CONT_A;
        }

        // Pause / Watch Menu (Tab / Escape) -> Start Button
        if (this.keys['Tab']) {
            buttons |= this.CONT_START;
        }

        // Gamepad API check (if connected)
        const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
        if (gamepads && gamepads[0]) {
            const gp = gamepads[0];
            // Left Stick = Move / Strafe
            if (Math.abs(gp.axes[0]) > 0.15) stickX = Math.round(gp.axes[0] * 80);
            if (Math.abs(gp.axes[1]) > 0.15) stickY = Math.round(-gp.axes[1] * 80);

            // Right Trigger (Fire)
            if (gp.buttons[7] && gp.buttons[7].pressed) buttons |= this.CONT_Z;
            // Left Trigger (Aim)
            if (gp.buttons[6] && gp.buttons[6].pressed) buttons |= this.CONT_R;
            // A Button / Cross (Interact / Reload)
            if (gp.buttons[0] && gp.buttons[0].pressed) buttons |= this.CONT_B;
            // X Button / Square (Cycle Weapon)
            if (gp.buttons[2] && gp.buttons[2].pressed) buttons |= this.CONT_A;
            // Start / Options (Pause)
            if (gp.buttons[9] && gp.buttons[9].pressed) buttons |= this.CONT_START;
        }

        this.buttons = buttons;
        this.stickX = stickX;
        this.stickY = stickY;

        return { buttons, stickX, stickY };
    }
}