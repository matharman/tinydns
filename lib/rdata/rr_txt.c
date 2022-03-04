#include "tiny_dns.h"

tiny_dns_err tiny_dns_parse_rdata_txt(IOReader *buf, struct tiny_dns_rr *rr) {
    char prefix;
    tiny_dns_err err = io_reader_get(buf, &prefix, 1);
    if (err < TINY_DNS_ERR_NONE) {
        return err;
    }

    const char *txt_start = NULL;
    err = io_reader_get_raw(buf, &txt_start, prefix);
    if (err < TINY_DNS_ERR_NONE) {
        return err;
    } else if (err != prefix) {
        return TINY_DNS_ERR_INVALID;
    }

    rr->rdata.rr_txt.txt = txt_start;
    rr->rdata.rr_txt.len = prefix;

    return err;
}
