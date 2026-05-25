#include "netplay_room_code.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/sf4e__NetUtil.hxx"

namespace sf4e {
namespace launcher {

	void FormatRoomCode(const char* host, uint16_t port, char* outCode, int outCodeLen) {
		snprintf(outCode, outCodeLen, "%s:%u", host, port);
	}

	bool ParseRoomCode(const char* roomCode, char* outHost, int outHostLen, uint16_t* outPort) {
		if (!roomCode || !outHost || !outPort) {
			return false;
		}
		char buf[256];
		strncpy_s(buf, roomCode, _TRUNCATE);
		sf4e::TrimRoomCodeInPlace(buf);
		if (!buf[0]) {
			return false;
		}

		char* colon = strrchr(buf, ':');
		if (!colon || colon == buf) {
			return false;
		}
		*colon = 0;
		const char* portStr = colon + 1;
		int port = atoi(portStr);
		if (port < 1 || port > 65535) {
			return false;
		}

		strncpy_s(outHost, outHostLen, buf, _TRUNCATE);
		if (!outHost[0]) {
			return false;
		}
		*outPort = (uint16_t)port;
		return true;
	}

} // namespace launcher
} // namespace sf4e
