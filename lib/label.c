#include "label.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum {
    LABEL_INVALID_PTR = -22,
    LABEL_SUCCESS = 0,
};

static inline bool is_label_ptr(const char *peek) {
    return *peek & 0xC0;
}

static inline size_t label_ptr_offset(const char ptr[2]) {
    uint8_t offset_hi = ptr[0] & ~0xC0;
    uint8_t offset_lo = ptr[1];
    return (offset_hi << 8) | offset_lo;
}

int tiny_dns_label_parse(IOWriter *wr, IOReader *rdr) {
    const char *origin = rdr->base;

    IOReader slicer;
    IOReader *active = rdr;

    int err = 0;

    while (true) {
        const char *raw;
        err = io_reader_get_raw(active, &raw, 1);
        if (err < IO_SUCCESS) {
            break;
        }

        // Root label
        if (*raw == 0) {
            err = io_writer_put(wr, raw, 1);
            if (err > IO_SUCCESS) {
                err = (int)wr->len;
            }
            break;
        }

        if (is_label_ptr(raw)) {
            char ptr[2] = { *raw, 0 };
            err = io_reader_get(active, &ptr[1], 1);
            if (err < IO_SUCCESS) {
                break;
            }

            size_t ptr_offset = label_ptr_offset(ptr);
            const char *label = origin + ptr_offset;
            size_t current_offset = active->ptr - origin;
            size_t label_len = current_offset - ptr_offset;

            io_reader_init(&slicer, label, label_len);
            active = &slicer;
            continue;
        }

        int label_len = (int)*raw;
        err = io_writer_put(wr, ".", 1);
        if (err < IO_SUCCESS) {
            break;
        }

        err = io_reader_get_raw(active, &raw, (size_t)label_len);
        if (err < IO_SUCCESS) {
            break;
        }

        err = io_writer_put(wr, raw, (size_t)label_len);
        if (err < IO_SUCCESS) {
            break;
        }
    }

    return err;
}
