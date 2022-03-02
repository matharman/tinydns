#ifndef TINY_DNS_IO_H
#define TINY_DNS_IO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IOError {
	IO_BUF_EMPTY = -1,
	IO_SUCCESS = 0,
};

typedef struct {
	const char *base;
	const char *ptr;
	size_t remaining;
} IOReader;

/// @brief Initialize an IOReader with a given buffer
///
/// @param rdr Pointer to uinitialized reader
/// @param data Buffer to wrap in the reader
/// @param len Length of @data in bytes
void io_reader_init(IOReader *rdr, const void *data, size_t len);

/// @brief Peek the next @len bytes of the reader
/// 	This consumes 0 bytes from the reader.
///
/// @param rdr Pointer to the reader in question
/// @param ptr Caller's pointer which will be set to the address of the reader's current offset
/// @param len Number of bytes to peek for @ptr
///
/// @return Number of bytes available to peek
/// @return <0 on error
int io_reader_peek_raw(IOReader *rdr, const char **ptr, size_t len);

/// @brief Get a raw pointer into the reader's current offset
/// 	This consumes @len bytes from the reader.
///
/// @param rdr Pointer to reader in question
/// @param ptr Caller's pointer, which will be set to the address of the reader's current offset
/// @param len Number of bytes to consume for @ptr
///
/// @return Number of bytes consumed on success
/// @return <0 on error
int io_reader_get_raw(IOReader *rdr, const char **ptr, size_t len);

/// @brief Copy @len bytes from the reader into @dest
///
/// @param rdr Pointer to reader in question
/// @param dest Caller's buffer where the copied bytes will be stored
/// @param len Number of bytes to consume from @rdr
///
/// @return Number of bytes consumed on success
/// @return <0 on error
int io_reader_get(IOReader *rdr, void *dest, size_t len);

/// @brief Consume 2 bytes from the reader, interpreting them as a BE uint16_t
///
/// @param rdr Pointer to reader in question
/// @param data Caller's pointer where the number will be copied
///
/// @return Number of bytes consumed on success
/// @return <0 on error
static inline int io_reader_get_u16(IOReader *rdr, uint16_t *data) {
	uint8_t bytes[sizeof(*data)];
	int err = io_reader_get(rdr, bytes, sizeof(bytes));
	*data = bytes[0] << 8 | bytes[1];
	return err;
}

/// @brief Consume 4 bytes from the reader, interpreting them as a BE uint32_t
///
/// @param rdr Pointer to reader in question
/// @param data Caller's pointer where the number will be copied
///
/// @return Number of bytes consumed on success
/// @return <0 on error
static inline int io_reader_get_u32(IOReader *rdr, uint32_t *data) {
	uint8_t bytes[sizeof(*data)];
	int err = io_reader_get(rdr, bytes, sizeof(bytes));
	*data = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
	return err;
}

#ifdef __cplusplus
}
#endif

#endif  // TINY_DNS_IO_H
