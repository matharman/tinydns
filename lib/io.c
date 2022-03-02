#include <string.h>

#include "io.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

void io_reader_init(IOReader *rdr, const void *data, size_t len) {
	rdr->base = data;
	rdr->remaining = len;
	rdr->ptr = rdr->base;
}

int io_reader_peek_raw(IOReader *rdr, const char **ptr, size_t len) {
	if(rdr->remaining == 0) {
		return IO_BUF_EMPTY;
	}

	*ptr = rdr->ptr;

	return (int)min(len, rdr->remaining);
}

int io_reader_get_raw(IOReader *rdr, const char **ptr, size_t len) {
	int consumed = io_reader_peek_raw(rdr, ptr, len);
	if(consumed < 0) {
		return consumed;
	}

	rdr->ptr += consumed;
	rdr->remaining -= consumed;
	return consumed;
}

int io_reader_get(IOReader *rdr, void *dest, size_t len) {
	const char *raw;
	int err = io_reader_get_raw(rdr, &raw, len);
	if (err < 0) {
		return err;
	}

	memcpy(dest, raw, err);

	return err;
}
