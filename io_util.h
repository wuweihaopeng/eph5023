#ifndef _IO_UTIL_H
#define _IO_UTIL_H

#define BOARD_ID_LEN 5
#define SUBBOARD_ID_LEN 4
#define SUBBOARD_ID_POS 16
#define MAP_ACCESS_ID_POS 48
#define MAP_ACCESS_OFFSET_BITS 12

#define NBIT_MASK(n) ((1LL << (n)) - 1)

#define OFFSET_ADDR_LEN (SUBBOARD_ID_POS + SUBBOARD_ID_LEN)
#define OFFSET_ADDR_MASK NBIT_MASK(OFFSET_ADDR_LEN)
#define BOARD_ID_MASK NBIT_MASK(BOARD_ID_LEN)
#define SUBBOARD_ID_MASK NBIT_MASK(SUBBOARD_ID_LEN)

#define MAP_BOARD_ID_POS (MAP_ACCESS_ID_POS + SUBBOARD_ID_LEN)
#define MAP_BASE_ADDR_LEN (MAP_ACCESS_ID_POS - MAP_ACCESS_OFFSET_BITS)//48 - 12 = 36
#define MAP_BASE_ADDR_MASK NBIT_MASK(MAP_BASE_ADDR_LEN)
#define MAP_OFFSET_ADDR_MASK NBIT_MASK(MAP_ACCESS_OFFSET_BITS)

#define MAP_PGSIZE (1 << MAP_ACCESS_OFFSET_BITS)
#define MAP_ACCESS_BASE (1 << MAP_ACCESS_OFFSET_BITS)

#define address_resolve(offset, board_id, off)                    \
    do {                                                          \
        board_id = ((offset) >> OFFSET_ADDR_LEN) & BOARD_ID_MASK; \
        off = (offset) &OFFSET_ADDR_MASK;                         \
    } while (0)

#define map_address_resolve(offset, board_id, subboard_id, base, off)     \
    do {                                                                  \
        board_id = ((offset) >> MAP_BOARD_ID_POS) & BOARD_ID_MASK;        \
        subboard_id = ((offset) >> MAP_ACCESS_ID_POS) & SUBBOARD_ID_MASK; \
        base = ((offset) >> MAP_ACCESS_OFFSET_BITS) & MAP_BASE_ADDR_MASK; \
        off = (offset) &MAP_OFFSET_ADDR_MASK;                             \
    } while (0)

#define map_offset_reform(addr, subboard_id, offset)                                                 \
    do {                                                                                             \
        addr = (subboard_id << SUBBOARD_ID_POS) | MAP_ACCESS_BASE | (offset & MAP_OFFSET_ADDR_MASK); \
    } while (0)

#define map_write_in_blocks(obj, id, write_block, base, offset, size, src)                      \
    do {                                                                                        \
        uint32_t seg_start = (offset);                                                          \
        uint32_t seg_size = min((size), MAP_PGSIZE - seg_start);                                \
        uint32_t sent_size = 0;                                                                 \
        uint64_t seg_base = (base);                                                             \
        for (sent_size = 0; sent_size < (size); sent_size += seg_size) {                        \
            error = (write_block) ((obj), (id), seg_base, seg_start, seg_size, src, sent_size); \
            if (error != PHY_IO_SUCCESS) {                                                      \
                printf("map_write_in_blocks error %d, offset %x, base %I64x.\n",                \
                       error, seg_start, seg_base);                                             \
                return error;                                                                   \
            }                                                                                   \
            seg_base++;                                                                         \
            seg_start = 0;                                                                      \
            seg_size = min((size) -sent_size, MAP_PGSIZE);                                      \
        }                                                                                       \
    } while (0)

#define map_read_from_blocks(obj, id, read_block, base, offset, size, dst)                     \
    do {                                                                                       \
        uint32_t seg_start = (offset);                                                         \
        uint32_t seg_size = min((size), MAP_PGSIZE - seg_start);                               \
        uint32_t read_size = 0;                                                                \
        uint64_t seg_base = (base);                                                            \
        for (read_size = 0; read_size < (size); read_size += seg_size) {                       \
            error = (read_block) ((obj), (id), seg_base, seg_start, seg_size, dst, read_size); \
            if (error != PHY_IO_SUCCESS) {                                                     \
                printf("map_read_from_blocks error %d, offset %x, base %I64x.\n",              \
                       error, seg_start, seg_base);                                            \
                return error;                                                                  \
            }                                                                                  \
            seg_base++;                                                                        \
            seg_start = 0;                                                                     \
            seg_size = min((size) -read_size, MAP_PGSIZE);                                     \
        }                                                                                      \
    } while (0)

#ifndef max
#define max(a, b) (((a) (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#endif