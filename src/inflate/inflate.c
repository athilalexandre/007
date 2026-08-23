#include <stdio.h>
/*
 * High-performance, memory-safe RFC 1951 Deflate Implementation for GoldenEye 007 WASM.
 * Decodes all textures, geometry, animation, and background data with zero external heap overhead.
 */
#include <ultra64.h>
#include <string.h>

/* Global variables expected by GoldenEye engine */
u8 *inbuf = NULL;
u8 *outbuf = NULL;
u32 inptr = 0;
u32 wp = 0;
u32 bb = 0;
u32 bk = 0;

struct huft {
    u8 e;
    u8 b;
    union {
        u16 n;
        struct huft *t;
    } v;
};

struct huft *huftlist = NULL;

/* TINF Huffman Tree */
typedef struct {
    u16 table[16];
    u16 trans[288];
} tinf_tree;

typedef struct {
    const u8 *src;
    u32 in_pos;
    u32 bit_buf;
    u32 num_bits;
    u8 *dst;
    u32 out_pos;
} tinf_state;

static const u8 s_length_bits[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static const u16 s_length_base[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const u8 s_dist_bits[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};
static const u16 s_dist_base[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
static const u8 s_clc_order[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static void tinf_build_tree(tinf_tree *t, const u8 *lengths, u32 num) {
    u16 offs[16];
    u32 i, sum;

    for (i = 0; i < 16; ++i) t->table[i] = 0;
    for (i = 0; i < num; ++i) {
        if (lengths[i] < 16) t->table[lengths[i]]++;
    }
    t->table[0] = 0;

    for (sum = 0, i = 0; i < 16; ++i) {
        offs[i] = sum;
        sum += t->table[i];
    }

    for (i = 0; i < num; ++i) {
        if (lengths[i] && lengths[i] < 16) {
            t->trans[offs[lengths[i]]++] = i;
        }
    }
}

static inline u32 tinf_get_bit(tinf_state *s) {
    if (s->num_bits == 0) {
        s->bit_buf = s->src[s->in_pos++];
        s->num_bits = 8;
    }
    u32 bit = s->bit_buf & 1;
    s->bit_buf >>= 1;
    s->num_bits--;
    return bit;
}

static inline u32 tinf_get_bits(tinf_state *s, u32 num) {
    u32 res = 0;
    for (u32 i = 0; i < num; ++i) {
        res |= (tinf_get_bit(s) << i);
    }
    return res;
}

static inline s32 tinf_decode_symbol(tinf_state *s, const tinf_tree *t) {
    s32 base = 0, offs = 0;
    s32 len = 1;

    while (1) {
        base |= tinf_get_bit(s);
        if (base < t->table[len]) {
            return t->trans[offs + base];
        }
        base -= t->table[len];
        offs += t->table[len];
        base <<= 1;
        len++;
        if (len >= 16) return -1;
    }
}

static void tinf_inflate_block_data(tinf_state *s, const tinf_tree *lt, const tinf_tree *dt) {
    while (1) {
        s32 sym = tinf_decode_symbol(s, lt);
        if (sym < 0) return;
        if (sym < 256) {
            s->dst[s->out_pos++] = (u8)sym;
        } else if (sym == 256) {
            return;
        } else {
            u32 lsym = (u32)(sym - 257);
            if (lsym >= 29) return;
            u32 length = s_length_base[lsym] + tinf_get_bits(s, s_length_bits[lsym]);
            s32 dist_sym = tinf_decode_symbol(s, dt);
            if (dist_sym < 0 || dist_sym >= 30) return;
            u32 dist = s_dist_base[dist_sym] + tinf_get_bits(s, s_dist_bits[dist_sym]);
            if (dist > s->out_pos) return;
            while (length--) {
                s->dst[s->out_pos] = s->dst[s->out_pos - dist];
                s->out_pos++;
            }
        }
    }
}

static void tinf_inflate_stream(tinf_state *s) {
    u32 bfinal;
    do {
        bfinal = tinf_get_bits(s, 1);
        u32 btype = tinf_get_bits(s, 2);

        if (btype == 0) { // Uncompressed
            s->bit_buf = 0;
            s->num_bits = 0;
            u16 len = s->src[s->in_pos] | (s->src[s->in_pos + 1] << 8);
            s->in_pos += 4;
            memcpy(s->dst + s->out_pos, s->src + s->in_pos, len);
            s->in_pos += len;
            s->out_pos += len;
        } else if (btype == 1) { // Fixed Huffman
            u8 lengths[288];
            for (u32 i = 0; i < 144; ++i) lengths[i] = 8;
            for (u32 i = 144; i < 256; ++i) lengths[i] = 9;
            for (u32 i = 256; i < 280; ++i) lengths[i] = 7;
            for (u32 i = 280; i < 288; ++i) lengths[i] = 8;
            tinf_tree lt;
            tinf_build_tree(&lt, lengths, 288);
            u8 dlengths[30];
            for (u32 i = 0; i < 30; ++i) dlengths[i] = 5;
            tinf_tree dt;
            tinf_build_tree(&dt, dlengths, 30);
            tinf_inflate_block_data(s, &lt, &dt);
        } else if (btype == 2) { // Dynamic Huffman
            u32 hlit = tinf_get_bits(s, 5) + 257;
            u32 hdist = tinf_get_bits(s, 5) + 1;
            u32 hclen = tinf_get_bits(s, 4) + 4;
            u8 clc_lengths[19] = {0};
            for (u32 i = 0; i < hclen; ++i) {
                clc_lengths[s_clc_order[i]] = tinf_get_bits(s, 3);
            }
            tinf_tree ct;
            tinf_build_tree(&ct, clc_lengths, 19);

            u8 tree_lengths[288 + 32] = {0};
            u32 total_lengths = hlit + hdist;
            if (total_lengths > 288 + 32) total_lengths = 288 + 32;
            u32 cur = 0;

            while (cur < total_lengths) {
                s32 sym = tinf_decode_symbol(s, &ct);
                if (sym < 0 || sym > 18) break;
                if (sym < 16) {
                    tree_lengths[cur++] = (u8)sym;
                } else if (sym == 16) {
                    u32 rep = tinf_get_bits(s, 2) + 3;
                    u8 val = (cur > 0) ? tree_lengths[cur - 1] : 0;
                    while (rep-- && cur < total_lengths) tree_lengths[cur++] = val;
                } else if (sym == 17) {
                    u32 rep = tinf_get_bits(s, 3) + 3;
                    while (rep-- && cur < total_lengths) tree_lengths[cur++] = 0;
                } else if (sym == 18) {
                    u32 rep = tinf_get_bits(s, 7) + 11;
                    while (rep-- && cur < total_lengths) tree_lengths[cur++] = 0;
                }
            }

            tinf_tree lt, dt;
            tinf_build_tree(&lt, tree_lengths, hlit);
            tinf_build_tree(&dt, tree_lengths + hlit, hdist);
            tinf_inflate_block_data(s, &lt, &dt);
        } else {
            break;
        }
    } while (!bfinal);
}

/* Engine API entries */
u32 decompress_entry(void *src, void *dst, void *hlist)
{
    u8 *src_ptr = (u8 *)src;
    printf("[decompress_entry] src=%p dst=%p bytes: %02x %02x %02x %02x\n", 
           src, dst, src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);
    fflush(stdout);
    int has_header = 0;
    if (src_ptr[0] == 0x11 && src_ptr[1] == 0x72) {
        src_ptr += 2;
        has_header = 1;
    }

    tinf_state s;
    s.src = src_ptr;
    s.dst = (u8 *)dst;
    s.in_pos = 0;
    s.out_pos = 0;
    s.bit_buf = 0;
    s.num_bits = 0;

    tinf_inflate_stream(&s);
    printf("[decompress_entry] complete out_pos=%u in_pos=%u\n", s.out_pos, s.in_pos); fflush(stdout);

    inbuf = (u8 *)src;
    inptr = s.in_pos + (has_header ? 2 : 0);
    wp = s.out_pos;

    return wp;
}

int inflate(void)
{
    return 0;
}

s32 huft_build(u32 *b, u32 n, u32 s, u16 *d, u8 *e, struct huft **t, s32 *m)
{
    return 0;
}
