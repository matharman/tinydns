/// @file tiny_dns.h
/// @brief Public API header for TinyDNS library

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

/// @brief Construct a DNS query in \p buffer
///
/// @param buffer Destination buffer for the serialized query.
/// @param len input: size of \p buffer in bytes, output: length of the query in bytes
/// @param id ID number to use for the query. Application specific, not used by the protocol.
/// @param name Hostname to resolve
/// @param qtype Record type to request, usually A or AAAA.
///
/// @return TINY_DNS_ERR_NONE on success
/// @return TINY_DNS_ERR_INVALID if parameters are NULL
/// @return TINY_DNS_ERR_NO_BUF if \p buffer is too small for the serialized query
tiny_dns_err tiny_dns_build_query(void *buffer, size_t *len, uint16_t id, const char *name,
                                  enum tiny_dns_rr_type qtype);

struct tiny_dns_iter {
    struct tiny_dns_header header;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    // TODO hide this detail somehow
    IOReader buf;
};

/// @brief Initialize a DNS response iterator over \p data
///     The provided data is assumed to be a response to a DNS query.
///     This function will parse the response header and discard the questions section.
///     In simple use cases, one already knows the question asked, so that section is not useful
///     when handling the response.
///     TODO the return values expose io.h errors, instead of mapping them to TINY_DNS_ERR values.
///
/// @param iter Pointer to uninitialized iterator
/// @param data Buffer containing the DNS response
/// @param len Length of @data in bytes
///
/// @return TINY_DNS_ERR_NONE on success
/// @return <TINY_DNS_ERR_NONE on error. This will be improved in future versions.
tiny_dns_err tiny_dns_iter_init(struct tiny_dns_iter *iter, void *data, size_t len);

/// @brief Parse the next resource record in wrapped by \p iter
///     In general, one should use \a tiny_dns_iter_foreach instead of this function to avoid
///     writing boilerplate. However, it is public for lower level use cases that don't want the
///     overhead of a callback.
///
///     This function will mutate the iterator to track how many records remain in each section. The
///     iterator object should not be accessed directly for this reason. The buffer wrapped by the
///     iterator object is not mutated, however.
///
/// @param iter Pointer to DNS response iterator object
/// @param rr Pointer to an empty resource record, where the result of decoding the next record will
///           be stored.
/// @param section Output pointer which will be set to the current section.
///
/// @return TINY_DNS_ERR_NONE on success
/// @return TINY_DNS_ERR_NO_BUF when the iterator is exhausted
/// @return <TINY_DNS_ERR_NONE on error
tiny_dns_err tiny_dns_iter_yield(struct tiny_dns_iter *iter, struct tiny_dns_rr *rr,
                                 enum tiny_dns_section *section);

typedef void (*tiny_dns_iter_fn)(struct tiny_dns_iter *iter, const struct tiny_dns_rr *rr,
                                 enum tiny_dns_section section, void *context);

/// @brief A convenience wrapper around \a tiny_dns_iter_yield.
///     This function invokes \p foreach_callback as the body of a for loop as \p iter is exhausted.
///     Every resource record in the response will be passed to the callback, so the user can handle
///     each record in any fashion they wish.
///
///     See the tiny_dns_cli for usage example.
///
/// @param iter Pointer to DNS response iterator object
/// @param foreach_callback User callback invoked as the "body" of the iteration
/// @param context User context for the callback.
///
/// @return TINY_DNS_ERR_NONE when the iterator is exhausted
/// @return <TINY_DNS_ERR_NONE on any error
tiny_dns_err tiny_dns_iter_foreach(struct tiny_dns_iter *iter, tiny_dns_iter_fn foreach_callback,
                                   void *context);

#ifdef __cplusplus
}
#endif

#endif  // TINY_DNS_H
