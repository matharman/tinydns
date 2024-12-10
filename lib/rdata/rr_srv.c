#include "io.h"
#include "rdata.h"
#include "tiny_dns.h"

tiny_dns_err tiny_dns_parse_rdata_srv(IOReader *buf, struct tiny_dns_rr *rr) {
    int err = io_reader_get_u16(buf, &rr->rdata.rr_srv.priority);
    if (err < IO_SUCCESS) {
        return err;
    }

    err = io_reader_get_u16(buf, &rr->rdata.rr_srv.weight);
    if (err < IO_SUCCESS) {
        return err;
    }

    err = io_reader_get_u16(buf, &rr->rdata.rr_srv.port);
    if (err < IO_SUCCESS) {
        return err;
    }

    return tiny_dns_name_decode(&rr->rdata.rr_srv.target, buf);
}
