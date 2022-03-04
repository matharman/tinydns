#include <cstring>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

#include "label.h"

TEST(LabelNormal, label_simple) {
    std::string raw_data = "\x4"
                           "abcd";
    raw_data.push_back(0);

    ASSERT_EQ(raw_data.length(), 6);

    IOReader rdr;
    io_reader_init(&rdr, raw_data.data(), raw_data.length());

    std::vector<char> label_out;
    label_out.reserve(32);

    IOWriter wr;
    io_writer_init(&wr, label_out.data(), label_out.capacity());

    int err = tiny_dns_label_parse(&wr, &rdr);
    ASSERT_EQ(6, err);
    ASSERT_EQ(0, std::memcmp(".abcd\0", label_out.data(), err));
}

TEST(Label, label_compoud) {
    std::string raw_data = "\x3"
                           "com";
    raw_data.push_back('\x00');
    raw_data.append("\x4"
                    "abcd");
    raw_data.push_back('\xC0');
    raw_data.push_back('\x00');

    ASSERT_EQ(raw_data.length(), 12);
    ASSERT_EQ(raw_data[10], '\xC0');

    IOReader rdr;
    io_reader_init(&rdr, raw_data.data(), raw_data.length());

    // Advance reader to the offset of the ptr-terminated label
    rdr.ptr += 5;
    rdr.remaining -= 5;
    ASSERT_EQ(*rdr.ptr, '\x4');

    std::vector<char> label_out;
    label_out.reserve(32);

    IOWriter wr;
    io_writer_init(&wr, label_out.data(), label_out.capacity());

    int err = tiny_dns_label_parse(&wr, &rdr);
    ASSERT_EQ(err, 10);
    ASSERT_EQ(wr.len, err);
    ASSERT_EQ(0, std::memcmp(".abcd.com", label_out.data(), wr.len));
}

TEST(Label, label_rfc_1035_sec_414_ex) {
    std::string raw_data = "\x1"
                           "f"
                           "\x3"
                           "isi"
                           "\x4"
                           "arpa";
    raw_data.push_back('\x00');
    raw_data.append("\x3"
                    "foo");
    raw_data.push_back('\xC0');
    raw_data.push_back('\x00');

    ASSERT_EQ(raw_data.length(), 18);

    IOReader rdr;
    io_reader_init(&rdr, raw_data.data(), raw_data.length());

    // Advance reader to the offset of the combo label
    rdr.ptr += 12;
    rdr.remaining -= 12;

    std::vector<char> label_out;
    label_out.reserve(128);

    IOWriter wr;
    io_writer_init(&wr, label_out.data(), label_out.capacity());

    int err = tiny_dns_label_parse(&wr, &rdr);
    ASSERT_EQ(err, 16);
    ASSERT_EQ(wr.len, err);
    ASSERT_EQ(0, std::memcmp(".foo.f.isi.arpa\0", label_out.data(), wr.len));
}
