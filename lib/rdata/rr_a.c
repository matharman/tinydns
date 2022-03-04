#include "tiny_dns.h"

tiny_dns_err tiny_dns_parse_rdata_a(IOReader *buf, struct tiny_dns_rr *rr) {
    return io_reader_get(buf, rr->rdata.rr_a, rr->rdlength);
}
