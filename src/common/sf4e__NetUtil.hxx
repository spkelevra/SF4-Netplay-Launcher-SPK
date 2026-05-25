#pragma once

namespace sf4e {

	bool DetectLanIPv4(char* outIp, int outIpLen);

	// Strips leading/trailing whitespace and line endings in place.
	void TrimRoomCodeInPlace(char* buf);

	// HTTPS lookup of public IPv4 (api.ipify.org, then icanhazip.com). timeoutMs per request.
	bool FetchPublicIPv4(char* outIp, int outIpLen, int timeoutMs = 5000);

	bool CopyTextToClipboardUtf8(const char* text);

} // namespace sf4e
