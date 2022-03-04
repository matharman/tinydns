#ifndef TINY_DNS_LABEL_H
#define TINY_DNS_LABEL_H

#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_MAX_LEN  253
#define LABEL_MAX_LEN 63

int tiny_dns_label_parse(IOWriter *dest, IOReader *rdr);

#ifdef __cplusplus
}
#endif

#endif  // TINY_DNS_LABEL_H
