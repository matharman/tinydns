#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

extern "C" {
#include "../lib/label.c"
}

TEST(LabelPtr, label_ptr_simple) {
    std::string raw_data = "\x4"
                           "abcd";
    raw_data.push_back('\x0');
    raw_data.push_back('\xC0');
    raw_data.push_back('\x00');

    ASSERT_EQ(raw_data.length(), 8);
    ASSERT_EQ(raw_data[6], '\xC0');

    IOReader rdr;
    io_reader_init(&rdr, raw_data.data(), raw_data.length());

    // Advance reader to the offset of the pointer
    rdr.ptr += 6;
    rdr.remaining -= 6;

    ASSERT_TRUE(is_label_ptr(rdr.ptr));
    ASSERT_EQ(label_ptr_offset(rdr.ptr), 0);

    std::vector<char> label_out;
    label_out.reserve(32);

    IOWriter wr;
    io_writer_init(&wr, label_out.data(), label_out.capacity());

    int err = tiny_dns_label_parse(&wr, &rdr);
    ASSERT_GT(err, IO_SUCCESS);
    ASSERT_STREQ(".abcd", label_out.data());

    // Check for inclusion of null terminator...
    ASSERT_EQ(label_out[5], '\0');
}

TEST(LabelPtr, label_ptr_long) {
    std::string raw_data = "\x4"
                           "abcd";
    raw_data.append("\x3"
                    "com");
    raw_data.push_back('\x0');
    raw_data.push_back('\xC0');
    raw_data.push_back('\x00');

    ASSERT_EQ(raw_data.length(), 12);
    ASSERT_EQ(raw_data[10], '\xC0');

    IOReader rdr;
    io_reader_init(&rdr, raw_data.data(), raw_data.length());

    // Advance reader to the offset of the pointer
    rdr.ptr += 10;
    rdr.remaining -= 10;

    ASSERT_TRUE(is_label_ptr(rdr.ptr));
    ASSERT_EQ(label_ptr_offset(rdr.ptr), 0);

    std::vector<char> label_out;
    label_out.reserve(32);

    IOWriter wr;
    io_writer_init(&wr, label_out.data(), label_out.capacity());

    int err = tiny_dns_label_parse(&wr, &rdr);
    ASSERT_GT(err, IO_SUCCESS);
    ASSERT_EQ(0, std::memcmp(wr.base, ".abcd.com\0", wr.len));

    // Check for inclusion of null terminator...
    ASSERT_EQ(label_out[wr.len - 1], '\0');
}
