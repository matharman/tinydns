#include <cstring>
#include <gtest/gtest.h>
#include <vector>

#include "io.h"

TEST(IOReaderTest, init) {
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    ASSERT_EQ(rdr.base, data.data());
    ASSERT_EQ(rdr.ptr, rdr.base);
    ASSERT_EQ(rdr.remaining, data.size());
}

TEST(IOReaderTest, peek_does_not_consume) {
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    const char *raw = nullptr;
    int err = io_reader_peek_raw(&rdr, &raw, 1);

    ASSERT_EQ(err, 1);
    ASSERT_NE(raw, nullptr);

    // Peek should not consume
    ASSERT_EQ(rdr.remaining, data.size());
    ASSERT_EQ(rdr.ptr, rdr.base);

    err = io_reader_peek_raw(&rdr, &raw, 1024);
    ASSERT_EQ(err, data.size());

    // Peek should not consume
    ASSERT_EQ(rdr.remaining, data.size());
    ASSERT_EQ(rdr.ptr, rdr.base);
}

TEST(IOReaderTest, get_raw) {
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    const char *raw = nullptr;
    int err = io_reader_get_raw(&rdr, &raw, 1);

    ASSERT_EQ(err, 1);
    ASSERT_EQ(raw, rdr.base);

    // get_* should consume
    ASSERT_EQ(rdr.remaining, data.size() - 1);
    ASSERT_EQ(rdr.ptr, rdr.base + 1);

    err = io_reader_get_raw(&rdr, &raw, 1024);
    ASSERT_EQ(err, data.size() - 1);
}

TEST(IOReaderTest, peek_follows_consume) {
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    const char *raw = nullptr;
    int err = io_reader_get_raw(&rdr, &raw, 1);

    ASSERT_EQ(err, 1);
    ASSERT_NE(raw, nullptr);

    // Peek should not consume
    ASSERT_EQ(rdr.remaining, data.size() - 1);
    ASSERT_EQ(rdr.ptr, rdr.base + 1);

    err = io_reader_peek_raw(&rdr, &raw, 1024);
    ASSERT_EQ(err, data.size() - 1);

    // Peek should not consume
    ASSERT_EQ(rdr.remaining, data.size() - 1);
    ASSERT_EQ(rdr.ptr, rdr.base + 1);
}

TEST(IOReaderTest, get) {
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    char dest[32];
    int err = io_reader_get(&rdr, dest, 1);

    ASSERT_EQ(err, 1);
    ASSERT_EQ(dest[0], data[0]);

    err = io_reader_get(&rdr, dest, sizeof(dest));
    ASSERT_EQ(err, 25);
    ASSERT_EQ(0, std::memcmp(dest, data.data() + 1, err));

    ASSERT_EQ(rdr.remaining, 0);
}

TEST(IOReaderTest, get_u16) {
    std::vector<uint8_t> data = { 0x00, 0x01 };
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    uint16_t value = 0xFFFF;
    int err = io_reader_get_u16(&rdr, &value);

    ASSERT_EQ(err, sizeof(value));
    ASSERT_EQ(value, 0x0001);
}

TEST(IOReaderTest, get_u32) {
    std::vector<uint8_t> data = { 0x00, 0x01, 0x02, 0x03 };
    IOReader rdr;
    io_reader_init(&rdr, data.data(), data.size());

    uint32_t value = 0xFFFFFFFF;
    int err = io_reader_get_u32(&rdr, &value);

    ASSERT_EQ(err, sizeof(value));
    ASSERT_EQ(value, 0x00010203);
}

TEST(IOReaderTest, exhausted_buffer) {
    IOReader rdr;
    io_reader_init(&rdr, nullptr, 0);

    int err = io_reader_get_raw(&rdr, nullptr, 32);
    ASSERT_EQ(err, IO_BUF_EMPTY);
}
