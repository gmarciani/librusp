#include "addrutil.h"

struct sockaddr_in createAddress(const char *ip, const int port) {
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;

	addr.sin_port = htons(port);

	if (!ip)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	else
		if (inet_pton(AF_INET, ip, &(addr.sin_addr)) <= 0)
			ERREXIT("Error in address to-network translation: %s:%d.", ip, port);

	return addr;
}

int isEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo) {
	return ((addrOne.sin_family == addrTwo.sin_family) &
			(addrOne.sin_port == addrTwo.sin_port) &
			(addrOne.sin_addr.s_addr == addrTwo.sin_addr.s_addr));
}

void addressToString(const struct sockaddr_in addr, char *buff) {
	char ip[INET_ADDRSTRLEN];
	int port;

	if (!inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	port = (int) ntohs(addr.sin_port);

	sprintf(buff, "%s:%d", ip, port);
}
