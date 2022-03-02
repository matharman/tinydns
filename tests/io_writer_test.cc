#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <cstring>

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

	for(auto i = mem.begin(); i != mem.begin() + 4; i++) {
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
