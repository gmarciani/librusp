#include "addrutil.h"

struct sockaddr_in createAddress(const char *ip, const int port) {
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (!ip) {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	} else {
		if (inet_pton(AF_INET, ip, &(addr.sin_addr)) <= 0) {
			fprintf(stderr, "Error in address to-network translation: %s:%d.\n", ip, port);
			exit(EXIT_FAILURE);
		}
	}

	return addr;
}

int isEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo) {
	return ((addrOne.sin_family == addrTwo.sin_family ) &&
			(addrOne.sin_port == addrTwo.sin_port) &&
			(addrOne.sin_addr.s_addr == addrTwo.sin_addr.s_addr));
}

char *getIp(const struct sockaddr_in addr) {
	char *str;

	if (!(str = malloc(INET_ADDRSTRLEN))) {
		fprintf(stderr, "Error in serialized address allocation.\n");
		exit(EXIT_FAILURE);
	}

	if (!inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN)) {
		fprintf(stderr, "Error in address to-presentation translation.\n");
		exit(EXIT_FAILURE);
	}

	return str;
}

int getPort(const struct sockaddr_in address) {
	return (int) ntohs(address.sin_port);
}
