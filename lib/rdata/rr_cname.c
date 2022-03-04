#include <string.h>

#include "label.h"
#include "rdata.h"
#include "tiny_dns.h"

tiny_dns_err tiny_dns_parse_rdata_cname(IOReader *buf, struct tiny_dns_rr *rr) {
    const char *raw;
    int err = io_reader_get_raw(buf, &raw, rr->rdlength);
    if (err < IO_SUCCESS) {
        return err;
    }

    IOReader cname;
    io_reader_init(&cname, raw, (size_t)err);
    cname.base = buf->base;

    err = tiny_dns_name_decode(&rr->rdata.rr_cname, &cname);
    if (err < TINY_DNS_ERR_NONE) {
        return err;
    }

    return err;
}
