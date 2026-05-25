#pragma once

#include <cstdint>

namespace sf4e {
namespace launcher {

	void FormatRoomCode(const char* host, uint16_t port, char* outCode, int outCodeLen);
	bool ParseRoomCode(const char* roomCode, char* outHost, int outHostLen, uint16_t* outPort);

} // namespace launcher
} // namespace sf4e
