#ifndef TINY_DNS_PRIV_H
#define TINY_DNS_PRIV_H

#include "tiny_dns.h"

#ifdef __cplusplus
extern "C" {
#endif

tiny_dns_err tiny_dns_name_decode(struct tiny_dns_name *name, IOReader *rdr);

tiny_dns_err tiny_dns_parse_rdata_a(IOReader *buf, struct tiny_dns_rr *rr);

tiny_dns_err tiny_dns_parse_rdata_aaaa(IOReader *buf, struct tiny_dns_rr *rr);

tiny_dns_err tiny_dns_parse_rdata_cname(IOReader *buf, struct tiny_dns_rr *rr);

tiny_dns_err tiny_dns_parse_rdata_srv(IOReader *buf, struct tiny_dns_rr *rr);

tiny_dns_err tiny_dns_parse_rdata_txt(IOReader *buf, struct tiny_dns_rr *rr);

#ifdef __cplusplus
}
#endif

#endif  // TINY_DNS_PRIV_H
