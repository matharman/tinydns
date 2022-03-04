#ifndef TINY_DNS_H
#define TINY_DNS_H

#include "io.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TINY_DNS_MAX_NAME_LEN  254
#define TINY_DNS_MAX_LABEL_LEN 64

typedef enum {
    TINY_DNS_ERR_RCODE = -3,
    TINY_DNS_ERR_NO_BUF = -2,
    TINY_DNS_ERR_INVALID = -1,
    TINY_DNS_ERR_NONE = 0,
} tiny_dns_err;

enum tiny_dns_class {
    CLASS_IN = 1,
};

enum tiny_dns_rr_type {
    RR_TYPE_A = 1,
    RR_TYPE_CNAME = 5,
    RR_TYPE_TXT = 16,
    RR_TYPE_AAAA = 28,
};

enum tiny_dns_opcode {
    OPCODE_QUERY = 0,
    OPCODE_IQUERY = 1,
    OPCODE_STATUS = 2,
};

enum tiny_dns_rcode {
    RCODE_NOERROR = 0,
    RCODE_FORMERR = 1,
    RCODE_SERVFAIL = 2,
    RCODE_NXDOMAIN = 3,
    RCODE_NOTIMP = 4,
    RCODE_REFUSED = 5,
    RCODE_YXDOMAIN = 6,
    RCODE_YXRRSET = 7,
    RCODE_NXRRSET = 8,
    RCODE_NOTAUTH = 9,
    RCODE_NOTZONE = 10,
};

enum tiny_dns_section {
    SECTION_ANSWER = 0,
    SECTION_AUTHORITY = 1,
    SECTION_ADDITIONAL = 2,
};

struct tiny_dns_flags {
    bool qr;
    enum tiny_dns_opcode opcode;
    bool aa;
    bool tc;
    bool rd;
    bool ra;
    bool ad;
    bool cd;
    enum tiny_dns_rcode rcode;
};

struct tiny_dns_header {
    uint16_t id;
    struct tiny_dns_flags flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct tiny_dns_name {
    char name[TINY_DNS_MAX_NAME_LEN];
    size_t len;
};

struct tiny_dns_question {
    struct tiny_dns_name qname;
    uint16_t qtype;
    uint16_t qclass;
};

struct tiny_dns_txt {
    const char *txt;
    uint8_t len;
};

struct tiny_dns_rr {
    struct tiny_dns_name name;
    uint16_t atype;
    uint16_t aclass;
    uint32_t ttl;
    uint16_t rdlength;
    union {
        uint8_t rr_a[4];
        uint8_t rr_aaaa[16];
        struct tiny_dns_name rr_cname;
        struct tiny_dns_txt rr_txt;
        struct {
            const char *data;
            size_t len;
        } unknown;
    } rdata;
};

tiny_dns_err tiny_dns_build_query(uint8_t *buffer, size_t *len, uint16_t id, const char *name,
                                  enum tiny_dns_rr_type qtype);

struct tiny_dns_iter {
    struct tiny_dns_header header;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    // TODO hide this detail somehow
    IOReader buf;
};

tiny_dns_err tiny_dns_iter_init(struct tiny_dns_iter *iter, void *data, size_t len);

tiny_dns_err tiny_dns_iter_yield(struct tiny_dns_iter *iter, struct tiny_dns_rr *rr,
                                 enum tiny_dns_section *section);

typedef void (*tiny_dns_iter_fn)(struct tiny_dns_iter *iter, const struct tiny_dns_rr *rr,
                                 enum tiny_dns_section section, void *context);

tiny_dns_err tiny_dns_iter_foreach(struct tiny_dns_iter *iter, tiny_dns_iter_fn foreach_callback,
                                   void *context);

#ifdef __cplusplus
}
#endif

#endif  // TINY_DNS_H
