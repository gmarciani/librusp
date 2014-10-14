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
	return ((addrOne.sin_family == addrTwo.sin_family ) &&
			(addrOne.sin_port == addrTwo.sin_port) &&
			(addrOne.sin_addr.s_addr == addrTwo.sin_addr.s_addr));
}

char *addressToString(const struct sockaddr_in addr) {
	char *straddr = NULL;
	char *strip = NULL;
	int port;

	strip = getIp(addr);

	port = getPort(addr);

	if (!(straddr = malloc(sizeof(char) * (ADDRESS_IPV4_MAX_OUTPUT + 1))))
		ERREXIT("Cannot allocate memory for address to string.");

	sprintf(straddr, "%s:%d", strip, port);

	free(strip);

	return straddr;

}

char *getIp(const struct sockaddr_in addr) {
	char *str = NULL;

	if (!(str = malloc(INET_ADDRSTRLEN)))
		ERREXIT("Cannot allocate memory for address string representation.");

	if (!inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	return str;
}

int getPort(const struct sockaddr_in address) {
	return (int) ntohs(address.sin_port);
}
