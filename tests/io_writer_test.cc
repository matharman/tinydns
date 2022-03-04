#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "io.h"

TEST(IOWriterTest, init) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    ASSERT_EQ(writer.base, mem.data());
    ASSERT_EQ(writer.capacity, mem.capacity());
    ASSERT_EQ(writer.ptr, mem.data());
    ASSERT_EQ(writer.len, 0);
}

TEST(IOWriter, claim) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    void *claim = nullptr;
    int err = io_writer_claim(&writer, &claim, 4);

    ASSERT_NE(claim, nullptr);
    ASSERT_EQ(err, 4);

    ASSERT_EQ(writer.ptr, writer.base + 4);
    ASSERT_EQ(writer.len, 4);

    memset(claim, 0xFF, err);

    for (auto i = mem.begin(); i != mem.begin() + 4; i++) {
        ASSERT_EQ(*i, '\xFF');
    }
}

TEST(IOWriter, put) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    int err = io_writer_put(&writer, "\x01\x02\x03\x04", 4);
    ASSERT_EQ(err, 4);

    ASSERT_EQ(writer.ptr, writer.base + 4);
    ASSERT_EQ(writer.len, 4);

    ASSERT_EQ(0, std::memcmp(mem.data(), "\x01\x02\x03\x04", 4));
}

TEST(IOWriter, put_multiple) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    int err = io_writer_put(&writer, "\x01\x02\x03\x04", 4);
    ASSERT_EQ(err, 4);

    ASSERT_EQ(writer.ptr, writer.base + 4);
    ASSERT_EQ(writer.len, 4);

    ASSERT_EQ(0, std::memcmp(mem.data(), "\x01\x02\x03\x04", 4));

    // Now do it again...
    err = io_writer_put(&writer, "\x01\x02\x03\x04", 4);
    ASSERT_EQ(err, 4);

    ASSERT_EQ(writer.ptr, writer.base + 8);
    ASSERT_EQ(writer.len, 8);

    ASSERT_EQ(0, std::memcmp(mem.data(), "\x01\x02\x03\x04", 4));
    ASSERT_EQ(0, std::memcmp(mem.data() + 4, "\x01\x02\x03\x04", 4));
}

TEST(IOWriter, put_u16) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    uint16_t val = 0x0102;
    int err = io_writer_put_u16(&writer, val);
    ASSERT_EQ(err, 2);
    ASSERT_EQ(0, std::memcmp(mem.data(), "\x01\x02", 2));
}

TEST(IOWriter, put_u32) {
    std::vector<char> mem;
    mem.reserve(8);

    IOWriter writer;
    io_writer_init(&writer, mem.data(), mem.capacity());

    uint32_t val = 0x01020304;
    int err = io_writer_put_u32(&writer, val);
    ASSERT_EQ(err, 4);
    ASSERT_EQ(0, std::memcmp(mem.data(), "\x01\x02\x03\x04", 4));
}
