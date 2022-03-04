#include <string.h>

#include "label.h"
#include "rdata.h"
#include "tiny_dns.h"

#define IS_ERR(_e) (_e < TINY_DNS_ERR_NONE)

#define DNS_HEADER_SIZE 12  // Always 12 bytes

//   1  1  1  1  1  1
//   5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |QR|   Opcode  |AA|TC|RD|RA| Z|AD|CD|   RCODE   |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
static int encode_header_flags(IOWriter *buf, const struct tiny_dns_flags *flags) {
    uint16_t bits = 0;

    if (flags->qr) {
        bits |= (1 << 15);
    }

    bits |= (flags->opcode << 11);

    if (flags->aa) {
        bits |= (1 << 10);
    }

    if (flags->tc) {
        bits |= (1 << 9);
    }

    if (flags->rd) {
        bits |= (1 << 8);
    }

    if (flags->ra) {
        bits |= (1 << 7);
    }

    if (flags->ad) {
        bits |= (1 << 5);
    }

    if (flags->cd) {
        bits |= (1 << 4);
    }

    bits |= (flags->rcode) & 0x0F;

    return io_writer_put_u16(buf, bits);
}

static tiny_dns_err tiny_dns_encode_header(IOWriter *buffer, const struct tiny_dns_header *hdr) {
    tiny_dns_err err = io_writer_put_u16(buffer, hdr->id);
    if (IS_ERR(err)) {
        return err;
    }

    err = encode_header_flags(buffer, &hdr->flags);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buffer, hdr->qdcount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buffer, hdr->ancount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buffer, hdr->nscount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buffer, hdr->arcount);
    if (IS_ERR(err)) {
        return err;
    }

    return err;
}

static tiny_dns_err tiny_dns_name_encode(IOWriter *buf, const char *name) {
    // leading length byte + string + null term
    size_t reserve_len = strlen(name) + 2;

    void *claim_raw;
    int err = io_writer_claim(buf, &claim_raw, reserve_len);
    if (err < IO_SUCCESS) {
        return err;
    }
    char *claim = claim_raw;

    // Domain names are composed of "labels".
    // Each label is separated by a "." in human-readable form.
    // In the DNS message, a label is proceeded by a single octet representing its
    // length, including the first label. Labels also have a null terminator.
    char *length_index = claim;
    claim++;

    // Include the null terminator, but not the leading byte
    size_t name_length = reserve_len - 1;
    memcpy(claim, name, name_length);

    for (size_t i = 0, j = 0; i < name_length; i++) {
        if (claim[i] == '.') {
            *length_index = (uint8_t)j;
            length_index = &claim[i];
            j = 0;
        } else if (claim[i] == '\0') {
            *length_index = (uint8_t)j;
        } else {
            j++;
        }
    }

    return TINY_DNS_ERR_NONE;
}

static tiny_dns_err tiny_dns_encode_question(IOWriter *buf, const char *name, uint16_t qclass,
                                             uint16_t qtype) {
    tiny_dns_err err = tiny_dns_name_encode(buf, name);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buf, qtype);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_writer_put_u16(buf, qclass);
    if (IS_ERR(err)) {
        return err;
    }

    return TINY_DNS_ERR_NONE;
}

tiny_dns_err tiny_dns_build_query(uint8_t *buffer, size_t *len, uint16_t id, const char *name,
                                  enum tiny_dns_rr_type qtype) {
    if (!buffer || !len || !name) {
        return TINY_DNS_ERR_INVALID;
    }

    IOWriter buf;
    io_writer_init(&buf, buffer, *len);

    struct tiny_dns_header header = {
        .id = id,
        .flags.opcode = OPCODE_QUERY,
        .flags.rd = true,
        .qdcount = 1,
    };

    tiny_dns_err err = tiny_dns_encode_header(&buf, &header);
    if (IS_ERR(err)) {
        return err;
    }

    err = tiny_dns_encode_question(&buf, name, CLASS_IN, qtype);
    if (IS_ERR(err)) {
        return err;
    }

    *len = buf.len;

    return TINY_DNS_ERR_NONE;
}

static void decode_flags(struct tiny_dns_flags *flags, uint16_t bits) {
    flags->qr = bits & (1 << 15);
    flags->opcode = (bits >> 11) & 0xF;
    flags->aa = bits & (1 << 10);
    flags->tc = bits & (1 << 9);
    flags->rd = bits & (1 << 8);
    flags->ra = bits & (1 << 7);
    flags->ad = bits & (1 << 5);
    flags->cd = bits & (1 << 4);
    flags->rcode = bits & 0x0F;
}

static tiny_dns_err tiny_dns_parse_header(struct tiny_dns_header *hdr, IOReader *buf) {
    tiny_dns_err err = io_reader_get_u16(buf, &hdr->id);
    if (IS_ERR(err)) {
        return err;
    }

    uint16_t bits = 0;
    err = io_reader_get_u16(buf, &bits);
    if (IS_ERR(err)) {
        return err;
    }

    decode_flags(&hdr->flags, bits);

    err = io_reader_get_u16(buf, &hdr->qdcount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &hdr->ancount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &hdr->nscount);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &hdr->arcount);
    if (IS_ERR(err)) {
        return err;
    }

    return TINY_DNS_ERR_NONE;
}

tiny_dns_err tiny_dns_name_decode(struct tiny_dns_name *name, IOReader *rdr) {
    IOWriter buf;
    io_writer_init(&buf, name->name, sizeof(name->name));

    int err = tiny_dns_label_parse(&buf, rdr);
    if (err < IO_SUCCESS) {
        return err;
    }

    // Trim the leading octet
    name->len = buf.len - 1;
    memmove(name->name, name->name + 1, name->len);

    return TINY_DNS_ERR_NONE;
}

static tiny_dns_err tiny_dns_parse_rdata_unknown(IOReader *buf, struct tiny_dns_rr *rr) {
    const char *raw;
    int err = io_reader_get_raw(buf, &raw, rr->rdlength);
    if (err > IO_SUCCESS) {
        rr->rdata.unknown.data = raw;
        rr->rdata.unknown.len = err;
    }

    return err;
}

static tiny_dns_err tiny_dns_parse_rr(struct tiny_dns_rr *rr, IOReader *buf) {
    tiny_dns_err err = tiny_dns_name_decode(&rr->name, buf);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &rr->atype);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &rr->aclass);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u32(buf, &rr->ttl);
    if (IS_ERR(err)) {
        return err;
    }

    err = io_reader_get_u16(buf, &rr->rdlength);
    if (IS_ERR(err)) {
        return err;
    }

    switch (rr->atype) {
        case RR_TYPE_A:
            err = tiny_dns_parse_rdata_a(buf, rr);
            break;
        case RR_TYPE_AAAA:
            err = tiny_dns_parse_rdata_aaaa(buf, rr);
            break;
        case RR_TYPE_CNAME:
            err = tiny_dns_parse_rdata_cname(buf, rr);
            break;
        case RR_TYPE_TXT:
            err = tiny_dns_parse_rdata_txt(buf, rr);
            break;
        default:
            err = tiny_dns_parse_rdata_unknown(buf, rr);
            break;
    }

    if (IS_ERR(err)) {
        return err;
    }

    return TINY_DNS_ERR_NONE;
}

static tiny_dns_err tiny_dns_discard_questions(uint16_t qdcount, IOReader *buf) {
    tiny_dns_err err = TINY_DNS_ERR_NONE;

    for (uint16_t i = 0; i < qdcount; i++) {
        struct tiny_dns_name discard = { 0 };
        err = tiny_dns_name_decode(&discard, buf);
        if (IS_ERR(err)) {
            break;
        }

        // Skip qclass & qtype
        uint16_t dummy;
        err = io_reader_get_u16(buf, &dummy);
        if (IS_ERR(err)) {
            break;
        }

        err = io_reader_get_u16(buf, &dummy);
        if (IS_ERR(err)) {
            break;
        }

        err = TINY_DNS_ERR_NONE;
    }

    return err;
}

tiny_dns_err tiny_dns_iter_init(struct tiny_dns_iter *iter, void *data, size_t len) {
    io_reader_init(&iter->buf, data, len);

    tiny_dns_err err = tiny_dns_parse_header(&iter->header, &iter->buf);
    if (IS_ERR(err)) {
        return err;
    }

    iter->ancount = iter->header.ancount;
    iter->nscount = iter->header.nscount;
    iter->arcount = iter->header.arcount;

    return tiny_dns_discard_questions(iter->header.qdcount, &iter->buf);
}

tiny_dns_err tiny_dns_iter_yield(struct tiny_dns_iter *iter, struct tiny_dns_rr *rr,
                                 enum tiny_dns_section *section) {
    tiny_dns_err err = tiny_dns_parse_rr(rr, &iter->buf);
    if (err == IO_BUF_EMPTY) {
        return TINY_DNS_ERR_NO_BUF;
    } else if (IS_ERR(err)) {
        return err;
    }

    if (iter->ancount) {
        *section = SECTION_ANSWER;
        iter->ancount--;
    } else if (iter->nscount) {
        *section = SECTION_AUTHORITY;
        iter->nscount--;
    } else if (iter->arcount) {
        *section = SECTION_ADDITIONAL;
        iter->arcount--;
    }

    return err;
}

tiny_dns_err tiny_dns_iter_foreach(struct tiny_dns_iter *iter, tiny_dns_iter_fn foreach_callback,
                                   void *context) {
    struct tiny_dns_rr rr;
    enum tiny_dns_section section;
    tiny_dns_err err = tiny_dns_iter_yield(iter, &rr, &section);

    for (; err == TINY_DNS_ERR_NONE; err = tiny_dns_iter_yield(iter, &rr, &section)) {
        if (foreach_callback) {
            foreach_callback(iter, &rr, section, context);
        }
    }

    // NO_BUF will be returned when the packet is exhausted, so iteration
    // succeeded
    if (err == TINY_DNS_ERR_NO_BUF) {
        err = TINY_DNS_ERR_NONE;
    }

    return err;
}
