#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "tiny_dns.h"

void hexdump(const char *label, const void *data, size_t len) {
	const uint8_t *buf = data;
	printf("%s\n", label);
	for (size_t i = 0; i < len; i += 2) {
		if (i > 0) {
			if (i % 16 == 0) {
				printf("\n");
			}
		}

		printf("%02x%02x ", buf[i], buf[i + 1]);
	}
	printf("\n");
}

static ssize_t query_send_recv(uint8_t *buffer, size_t len, size_t max,
			       const char *server) {
	int udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udp < 0) {
		perror("socket");
		return -errno;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(53);
	if (inet_pton(AF_INET, server, &addr.sin_addr) != 1) {
		perror("inet_pton");
		return -errno;
	}

	ssize_t err = sendto(udp, buffer, len, 0, (struct sockaddr *)&addr,
			     sizeof(addr));
	if (err < 0) {
		perror("sendto");
		return -errno;
	}

	err = recvfrom(udp, buffer, max, 0, NULL, 0);
	if (err < 0) {
		perror("recvfrom");
		return -errno;
	}

	close(udp);

	return err;
}

static int resolve_query(const char *nameservers[], uint8_t *buffer,
			 size_t *len, size_t max) {
	hexdump("Query------------", buffer, *len);

	size_t query_len = *len;
	*len = 0;

	for (const char *srv = nameservers[0]; srv != NULL; srv++) {
		printf("Querying server %s\n", srv);
		ssize_t err = query_send_recv(buffer, query_len, max, srv);
		if (err < 0) {
			continue;
		}

		*len = (size_t)err;
		break;
	}

	hexdump("Response------------", buffer, *len);

	return 0;
}

static enum tiny_dns_rr_type rr_type_from_str(const char *str) {
	if (strcmp(str, "aaaa") == 0 || strcmp(str, "AAAA") == 0) {
		return RR_TYPE_AAAA;
	} else if (strcmp(str, "txt") == 0 || strcmp(str, "TXT") == 0) {
		return RR_TYPE_TXT;
	}

	return RR_TYPE_A;
}

const char *section_str(enum tiny_dns_section section) {
	const char *section_str[] = {
		[SECTION_ANSWER] = "ANSWER",
		[SECTION_AUTHORITY] = "AUTHORITY",
		[SECTION_ADDITIONAL] = "ADDITIONAL",
	};

	return section_str[section];
}

static void print_cname(const struct tiny_dns_name *name) {
	hexdump("RR CNAME------", name->name, name->len);
	for (size_t i = 0; i < name->len; i++) {
		printf("%c", name->name[i]);
	}
	printf("\n");
}

static void rr_foreach(struct tiny_dns_iter *iter, const struct tiny_dns_rr *rr,
		       enum tiny_dns_section section, void *context) {
	printf("%s: ", section_str(section));

	char scratch[INET6_ADDRSTRLEN];
	switch (rr->atype) {
	case RR_TYPE_A:
		inet_ntop(AF_INET, rr->rdata.rr_a, scratch, sizeof(scratch));
		printf("RR A: %s\n", scratch);
		break;
	case RR_TYPE_AAAA:
		inet_ntop(AF_INET6, rr->rdata.rr_aaaa, scratch,
			  sizeof(scratch));
		printf("RR AAAA: %s\n", scratch);
		break;
	case RR_TYPE_CNAME:
		print_cname(&rr->rdata.rr_cname);
		break;
	case RR_TYPE_TXT:
		hexdump("RR TXT------", rr->rdata.rr_txt.txt,
			rr->rdata.rr_txt.len);
		break;
	default:
		printf("RR TYPE 0x%0X: ", rr->atype);
		hexdump("UKNOWN RR------", rr->rdata.unknown.data,
			rr->rdata.unknown.len);
		break;
	}
}

int main(int argc, char *argv[]) {
	uint8_t buffer[512] = { 0 };

	enum tiny_dns_rr_type qtype = RR_TYPE_A;
	if (argc == 4) {
		qtype = rr_type_from_str(argv[3]);
	}

	size_t len = sizeof(buffer);
	tiny_dns_err err =
		tiny_dns_build_query(buffer, &len, 0xdb42, argv[2], qtype);
	if (err != TINY_DNS_ERR_NONE) {
		printf("build query err: %d\n", err);
		return 1;
	}

	const char *nameservers[] = {
		argv[1],
		NULL,
	};

	resolve_query(nameservers, buffer, &len, sizeof(buffer));

	struct tiny_dns_iter iter;
	err = tiny_dns_iter_init(&iter, buffer, len);
	if (err != TINY_DNS_ERR_NONE) {
		printf("iter init error: %d\n", err);
		return 1;
	}

	// hexdump("after iter", buffer, len);

	err = tiny_dns_iter_foreach(&iter, rr_foreach, NULL);
	if (err != TINY_DNS_ERR_NONE) {
		printf("foreach error: %d\n", err);
		return 1;
	}

	// hexdump("after foreach", buffer, len);

	return 0;
}
