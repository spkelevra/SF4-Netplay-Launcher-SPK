#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winhttp.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

namespace sf4e {

	void TrimRoomCodeInPlace(char* buf) {
		if (!buf) {
			return;
		}
		size_t len = strlen(buf);
		size_t start = 0;
		while (start < len && (buf[start] == ' ' || buf[start] == '\t' || buf[start] == '\r' || buf[start] == '\n')) {
			start++;
		}
		size_t end = len;
		while (end > start && (buf[end - 1] == ' ' || buf[end - 1] == '\t' || buf[end - 1] == '\r' || buf[end - 1] == '\n')) {
			end--;
		}
		if (start > 0) {
			memmove(buf, buf + start, end - start);
		}
		buf[end - start] = '\0';
	}

	static bool HttpGetFirstLine(const wchar_t* host, const wchar_t* path, int timeoutMs, char* outBody, int outBodyLen) {
		if (!host || !path || !outBody || outBodyLen <= 0) {
			return false;
		}

		HINTERNET hSession = WinHttpOpen(
			L"sf4e/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		);
		if (!hSession) {
			return false;
		}

		WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

		HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
		if (!hConnect) {
			WinHttpCloseHandle(hSession);
			return false;
		}

		HINTERNET hRequest = WinHttpOpenRequest(
			hConnect,
			L"GET",
			path,
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE
		);
		if (!hRequest) {
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return false;
		}

		BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		if (!ok || !WinHttpReceiveResponse(hRequest, NULL)) {
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return false;
		}

		DWORD status = 0;
		DWORD statusSize = sizeof(status);
		WinHttpQueryHeaders(
			hRequest,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&status,
			&statusSize,
			WINHTTP_NO_HEADER_INDEX
		);
		if (status != 200) {
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return false;
		}

		int total = 0;
		outBody[0] = '\0';
		for (;;) {
			DWORD avail = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &avail) || avail == 0) {
				break;
			}
			if (total + (int)avail >= outBodyLen - 1) {
				avail = (DWORD)(outBodyLen - 1 - total);
			}
			DWORD read = 0;
			if (!WinHttpReadData(hRequest, outBody + total, avail, &read) || read == 0) {
				break;
			}
			total += (int)read;
			outBody[total] = '\0';
			if (total >= outBodyLen - 1) {
				break;
			}
		}

		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);

		TrimRoomCodeInPlace(outBody);
		return total > 0;
	}

	bool FetchPublicIPv4(char* outIp, int outIpLen, int timeoutMs) {
		if (!outIp || outIpLen <= 0) {
			return false;
		}

		char body[64] = { 0 };
		if (HttpGetFirstLine(L"api.ipify.org", L"/", timeoutMs, body, sizeof(body))) {
			strncpy_s(outIp, outIpLen, body, _TRUNCATE);
			return true;
		}

		memset(body, 0, sizeof(body));
		if (HttpGetFirstLine(L"icanhazip.com", L"/", timeoutMs, body, sizeof(body))) {
			strncpy_s(outIp, outIpLen, body, _TRUNCATE);
			return true;
		}

		return false;
	}

	bool CopyTextToClipboardUtf8(const char* text) {
		if (!text || !text[0]) {
			return false;
		}

		int wideLen = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
		if (wideLen <= 0) {
			return false;
		}

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideLen * sizeof(wchar_t));
		if (!hMem) {
			return false;
		}

		wchar_t* wide = (wchar_t*)GlobalLock(hMem);
		if (!wide) {
			GlobalFree(hMem);
			return false;
		}
		MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, wideLen);
		GlobalUnlock(hMem);

		if (!OpenClipboard(NULL)) {
			GlobalFree(hMem);
			return false;
		}
		EmptyClipboard();
		BOOL set = SetClipboardData(CF_UNICODETEXT, hMem) != NULL;
		CloseClipboard();
		if (!set) {
			GlobalFree(hMem);
		}
		return set != FALSE;
	}

	bool DetectLanIPv4(char* outIp, int outIpLen) {
		if (!outIp || outIpLen <= 0) {
			return false;
		}

		ULONG bufLen = 15000;
		PIP_ADAPTER_ADDRESSES addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
		if (!addrs) {
			strncpy_s(outIp, outIpLen, "127.0.0.1", _TRUNCATE);
			return false;
		}

		ULONG res = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, addrs, &bufLen);
		if (res == ERROR_BUFFER_OVERFLOW) {
			free(addrs);
			addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
			if (!addrs) {
				strncpy_s(outIp, outIpLen, "127.0.0.1", _TRUNCATE);
				return false;
			}
			res = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, addrs, &bufLen);
		}

		bool found = false;
		if (res == NO_ERROR) {
			for (PIP_ADAPTER_ADDRESSES a = addrs; a; a = a->Next) {
				if (a->OperStatus != IfOperStatusUp) {
					continue;
				}
				for (PIP_ADAPTER_UNICAST_ADDRESS u = a->FirstUnicastAddress; u; u = u->Next) {
					sockaddr_in* sa = (sockaddr_in*)u->Address.lpSockaddr;
					char buf[32];
					InetNtopA(AF_INET, &sa->sin_addr, buf, sizeof(buf));
					if (strncmp(buf, "127.", 4) == 0) {
						continue;
					}
					if (strncmp(buf, "169.254.", 8) == 0) {
						continue;
					}
					strncpy_s(outIp, outIpLen, buf, _TRUNCATE);
					found = true;
					break;
				}
				if (found) {
					break;
				}
			}
		}

		free(addrs);
		if (!found) {
			strncpy_s(outIp, outIpLen, "127.0.0.1", _TRUNCATE);
		}
		return found;
	}

} // namespace sf4e
