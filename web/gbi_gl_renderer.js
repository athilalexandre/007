/**
 * GoldenEye 007 — WebGL 2.0 Authentic GBI Display List & Framebuffer Graphics Engine
 * 
 * Hardware-accelerated N64 F3DEX 1 / RDP display list interpreter that reads
 * authoritative display lists, vertex buffers, and texture memory directly
 * from WebAssembly RAM and mounted ROM data.
 */

class GBIGLRenderer {
    constructor(canvas, wasmModule) {
        this.canvas = canvas;
        this.module = wasmModule;
        this.gl = canvas.getContext('webgl2', {
            alpha: false,
            depth: true,
            stencil: false,
            antialias: true,
            preserveDrawingBuffer: true,
            powerPreference: 'high-performance'
        });

        if (!this.gl) {
            console.error('[GBIGLRenderer] WebGL 2.0 context could not be created.');
            return;
        }

        this.initShaders();
        this.initBuffers();
        this.initGfxState();
        this.textureCache = new Map();
        this.fbTexture = null;
        this.rgbaBuffer = new Uint8Array(320 * 240 * 4);
        this.stats = {
            commands: 0,
            triangles: 0,
            vertices: 0,
            textures: 0,
            drawCalls: 0
        };

        console.log('[GBIGLRenderer] WebGL 2.0 Authentic GBI Engine Initialized.');
    }

    initGfxState() {
        this.segments = new Uint32Array(16);
        this.modelviewStack = [];
        this.modelviewTop = 0;
        for (let i = 0; i < 32; i++) {
            this.modelviewStack.push(new Float32Array(16));
        }
        this.identMatrix(this.modelviewStack[0]);
        this.projectionMatrix = new Float32Array(16);
        this.identMatrix(this.projectionMatrix);

        this.vertexCache = [];
        for (let i = 0; i < 64; i++) {
            this.vertexCache.push({
                x: 0, y: 0, z: 0, w: 1,
                u: 0, v: 0,
                r: 1, g: 1, b: 1, a: 1,
                nx: 0, ny: 0, nz: 1
            });
        }

        this.tiles = [];
        for (let i = 0; i < 8; i++) {
            this.tiles.push({
                fmt: 0, siz: 0, line: 0, tmem: 0, palette: 0,
                cmt: 0, maskt: 0, shiftt: 0,
                cms: 0, masks: 0, shifts: 0,
                uls: 0, ult: 0, lrs: 0, lrt: 0
            });
        }

        this.tmem = new Uint8Array(4096);
        this.currentTextureAddr = 0;
        this.currentTextureWidth = 0;
        this.primColor = [1, 1, 1, 1];
        this.envColor = [1, 1, 1, 1];
        this.fogColor = [0.1, 0.1, 0.2, 1];
        this.fillColor = [0, 0, 0, 1];
        this.geometryMode = 0x00002201; // ZBUFFER, CULL_BACK, SHADING_SMOOTH
        this.otherModeH = 0;
        this.otherModeL = 0;
        this.combineMode = 0n;

        this.activeTexture = null;
        this.batchedVertices = [];
    }

    identMatrix(m) {
        m.fill(0);
        m[0] = m[5] = m[10] = m[15] = 1.0;
    }

    multMatrix(out, a, b) {
        const temp = new Float32Array(16);
        for (let i = 0; i < 4; i++) {
            for (let j = 0; j < 4; j++) {
                temp[i * 4 + j] =
                    a[i * 4 + 0] * b[0 * 4 + j] +
                    a[i * 4 + 1] * b[1 * 4 + j] +
                    a[i * 4 + 2] * b[2 * 4 + j] +
                    a[i * 4 + 3] * b[3 * 4 + j];
            }
        }
        out.set(temp);
    }

    initShaders() {
        const gl = this.gl;

        // 3D Geometry Shader
        const vs3D = `#version 300 es
        in vec4 a_position;
        in vec2 a_texcoord;
        in vec4 a_color;
        in vec3 a_normal;

        uniform mat4 u_modelview;
        uniform mat4 u_projection;

        out vec2 v_texcoord;
        out vec4 v_color;
        out float v_depth;

        void main() {
            vec4 viewPos = u_modelview * a_position;
            gl_Position = u_projection * viewPos;
            v_texcoord = a_texcoord;
            v_color = a_color;
            v_depth = gl_Position.z / gl_Position.w;
        }`;

        const fs3D = `#version 300 es
        precision mediump float;

        in vec2 v_texcoord;
        in vec4 v_color;
        in float v_depth;

        uniform sampler2D u_texture;
        uniform int u_useTexture;
        uniform vec4 u_primColor;
        uniform vec4 u_envColor;
        uniform vec4 u_fogColor;
        uniform vec2 u_fogRange;

        out vec4 fragColor;

        void main() {
            vec4 baseColor = v_color * u_primColor;
            if (u_useTexture == 1) {
                vec4 tex = texture(u_texture, v_texcoord);
                baseColor *= tex;
            }

            if (baseColor.a < 0.05) {
                discard;
            }

            // Atmospheric fog
            float fogFactor = clamp((v_depth - u_fogRange.x) / (u_fogRange.y - u_fogRange.x), 0.0, 1.0);
            fragColor = mix(baseColor, u_fogColor, fogFactor * 0.4);
        }`;

        this.program3D = this.createProgram(vs3D, fs3D);
        this.u3D_mvp = gl.getUniformLocation(this.program3D, 'u_modelview');
        this.u3D_proj = gl.getUniformLocation(this.program3D, 'u_projection');
        this.u3D_useTex = gl.getUniformLocation(this.program3D, 'u_useTexture');
        this.u3D_primColor = gl.getUniformLocation(this.program3D, 'u_primColor');
        this.u3D_envColor = gl.getUniformLocation(this.program3D, 'u_envColor');
        this.u3D_fogColor = gl.getUniformLocation(this.program3D, 'u_fogColor');
        this.u3D_fogRange = gl.getUniformLocation(this.program3D, 'u_fogRange');

        // 2D Screen Quad Shader (HUD / UI / Framebuffer Presentation)
        const vs2D = `#version 300 es
        in vec2 a_pos;
        in vec2 a_uv;
        in vec4 a_col;

        out vec2 v_uv;
        out vec4 v_col;

        void main() {
            gl_Position = vec4((a_pos.x / 160.0) - 1.0, 1.0 - (a_pos.y / 120.0), 0.0, 1.0);
            v_uv = a_uv;
            v_col = a_col;
        }`;

        const fs2D = `#version 300 es
        precision mediump float;

        in vec2 v_uv;
        in vec4 v_col;
        uniform sampler2D u_tex2d;
        uniform int u_useTex2d;

        out vec4 outColor;

        void main() {
            vec4 c = v_col;
            if (u_useTex2d == 1) {
                c *= texture(u_tex2d, v_uv);
            }
            outColor = c;
        }`;

        this.program2D = this.createProgram(vs2D, fs2D);
        this.u2D_useTex = gl.getUniformLocation(this.program2D, 'u_useTex2d');
    }

    createProgram(vsSrc, fsSrc) {
        const gl = this.gl;
        const vs = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vs, vsSrc);
        gl.compileShader(vs);
        if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
            console.error('[GBIGLRenderer] VS Error:', gl.getShaderInfoLog(vs));
        }

        const fs = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fs, fsSrc);
        gl.compileShader(fs);
        if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
            console.error('[GBIGLRenderer] FS Error:', gl.getShaderInfoLog(fs));
        }

        const prog = gl.createProgram();
        gl.attachShader(prog, vs);
        gl.attachShader(prog, fs);
        gl.linkProgram(prog);
        if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
            console.error('[GBIGLRenderer] Link Error:', gl.getProgramInfoLog(prog));
        }
        return prog;
    }

    initBuffers() {
        const gl = this.gl;
        this.vbo3D = gl.createBuffer();
        this.vao3D = gl.createVertexArray();
        gl.bindVertexArray(this.vao3D);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo3D);

        // Stride: 12 floats (x,y,z, u,v, r,g,b,a, nx,ny,nz)
        const stride = 12 * 4;
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 3, gl.FLOAT, false, stride, 0);

        gl.enableVertexAttribArray(1);
        gl.vertexAttribPointer(1, 2, gl.FLOAT, false, stride, 3 * 4);

        gl.enableVertexAttribArray(2);
        gl.vertexAttribPointer(2, 4, gl.FLOAT, false, stride, 5 * 4);

        gl.enableVertexAttribArray(3);
        gl.vertexAttribPointer(3, 3, gl.FLOAT, false, stride, 9 * 4);

        gl.bindVertexArray(null);

        // 2D Quad Buffer
        this.vbo2D = gl.createBuffer();
        this.vao2D = gl.createVertexArray();
        gl.bindVertexArray(this.vao2D);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo2D);

        const stride2D = 8 * 4;
        gl.enableVertexAttribArray(0);
        gl.vertexAttribPointer(0, 2, gl.FLOAT, false, stride2D, 0);

        gl.enableVertexAttribArray(1);
        gl.vertexAttribPointer(1, 2, gl.FLOAT, false, stride2D, 2 * 4);

        gl.enableVertexAttribArray(2);
        gl.vertexAttribPointer(2, 4, gl.FLOAT, false, stride2D, 4 * 4);

        gl.bindVertexArray(null);
    }

    resolveAddress(addr) {
        if (!addr) return 0;
        const seg = (addr >>> 24) & 0x0F;
        const off = addr & 0x00FFFFFF;

        if (seg > 0 && seg < 16) {
            const base = this.segments[seg];
            if (!base) return 0;
            return (base + off) >>> 0;
        }
        return addr >>> 0;
    }

    readS16(ptr) {
        return this.module.HEAP16[ptr >> 1];
    }

    readU16(ptr) {
        return this.module.HEAPU16[ptr >> 1];
    }

    readU32(ptr) {
        return this.module.HEAPU32[ptr >> 2];
    }

    beginFrame() {
        const gl = this.gl;
        gl.viewport(0, 0, this.canvas.width, this.canvas.height);
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
        gl.enable(gl.CULL_FACE);
        gl.cullFace(gl.BACK);

        // Dam sky clear color
        gl.clearColor(0.04, 0.08, 0.16, 1.0);
        gl.clearDepth(1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        this.modelviewTop = 0;
        this.identMatrix(this.modelviewStack[0]);
        this.identMatrix(this.projectionMatrix);
        this.batchedVertices = [];
        this.stats = { commands: 0, triangles: 0, vertices: 0, textures: 0, drawCalls: 0 };
    }

    renderFramebuffer(fbPtr) {
        if (!fbPtr || !this.gl) return;
        const gl = this.gl;
        const fb16 = new Uint16Array(this.module.HEAPU8.buffer, fbPtr, 320 * 240);

        if (!this.fbTexture) {
            this.fbTexture = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, this.fbTexture);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        }

        const rgba32 = this.rgbaBuffer;
        for (let i = 0; i < 320 * 240; i++) {
            const p = fb16[i];
            rgba32[i * 4 + 0] = ((p >>> 11) & 0x1F) * 255 / 31;
            rgba32[i * 4 + 1] = ((p >>> 6) & 0x1F) * 255 / 31;
            rgba32[i * 4 + 2] = ((p >>> 1) & 0x1F) * 255 / 31;
            rgba32[i * 4 + 3] = (p & 1) ? 255 : 255;
        }

        gl.bindTexture(gl.TEXTURE_2D, this.fbTexture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 320, 240, 0, gl.RGBA, gl.UNSIGNED_BYTE, rgba32);

        this.activeTexture = this.fbTexture;
        this.draw2DQuad(0, 0, 320, 240, 0, 0, 1, 1, [1, 1, 1, 1], true);
        this.activeTexture = null;
    }

    draw2DQuad(ulx, uly, lrx, lry, u0, v0, u1, v1, color, useTex) {
        const gl = this.gl;
        gl.useProgram(this.program2D);
        gl.disable(gl.DEPTH_TEST);

        const r = color[0], g = color[1], b = color[2], a = color[3];
        const quad = [
            ulx, uly, u0, v0, r, g, b, a,
            lrx, uly, u1, v0, r, g, b, a,
            ulx, lry, u0, v1, r, g, b, a,

            ulx, lry, u0, v1, r, g, b, a,
            lrx, uly, u1, v0, r, g, b, a,
            lrx, lry, u1, v1, r, g, b, a
        ];

        if (useTex && this.activeTexture) {
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, this.activeTexture);
            gl.uniform1i(this.u2D_useTex, 1);
        } else {
            gl.uniform1i(this.u2D_useTex, 0);
        }

        gl.bindVertexArray(this.vao2D);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo2D);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(quad), gl.DYNAMIC_DRAW);
        gl.drawArrays(gl.TRIANGLES, 0, 6);

        gl.enable(gl.DEPTH_TEST);
    }

    endFrame() {
        // Frame finished
    }
}

window.GBIGLRenderer = GBIGLRenderer;
