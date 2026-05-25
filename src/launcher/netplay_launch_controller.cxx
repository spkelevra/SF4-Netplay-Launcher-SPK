#include "netplay_launch_controller.hxx"



#include <stdio.h>

#include <stdlib.h>

#include <string.h>



#include <windows.h>

#include <pathcch.h>
#include <shlobj.h>
#include <strsafe.h>



#include "../common/sf4e__NetUtil.hxx"

#include "connect_strategy.hxx"

#include "netplay_room_code.hxx"



namespace sf4e {

namespace launcher {



	static const int kProtocolVersion = 1;



	NetplayLaunchController::NetplayLaunchController(PersistedSettings& settings, NetplayConfig& outConfig)

		: m_settings(settings), m_outConfig(outConfig) {

		m_lanIp[0] = 0;

		sf4e::DetectLanIPv4(m_lanIp, sizeof(m_lanIp));

	}



	nlohmann::json NetplayLaunchController::MakeStateEnvelope() const {

		nlohmann::json j;

		j["v"] = kProtocolVersion;

		j["type"] = "state";

		return j;

	}



	std::string NetplayLaunchController::PreviewRoomCode(const char* shareIp, uint16_t port) const {

		char buf[128];

		if (!shareIp || !shareIp[0]) {

			return "(enter internet address)";

		}

		FormatRoomCode(shareIp, port, buf, sizeof(buf));

		return buf;

	}



	nlohmann::json NetplayLaunchController::BuildStateJson() const {

		nlohmann::json j = MakeStateEnvelope();

		j["displayName"] = m_settings.displayName;

		j["inputDelay"] = m_settings.inputDelay;

		j["sessionPort"] = m_settings.sessionPort;

		j["ggpoPort"] = m_settings.ggpoPort;

		j["lanIp"] = m_lanIp;

		j["lastJoinHost"] = m_settings.lastJoinHost;

		j["advertiseHost"] = m_settings.lastAdvertiseHost[0] ? m_settings.lastAdvertiseHost : m_lanIp;

		j["roomCodePreview"] = PreviewRoomCode(

			m_settings.lastAdvertiseHost[0] ? m_settings.lastAdvertiseHost : m_lanIp,

			m_settings.sessionPort

		);

		j["lanRoomCode"] = PreviewRoomCode(m_lanIp, m_settings.sessionPort);

		j["featureFlags"] = {

			{ "matchmaking", false },

			{ "autoNat", false }

		};

		return j;

	}



	nlohmann::json NetplayLaunchController::HandleWebMessage(const nlohmann::json& msg) {

		const std::string type = msg.value("type", "");

		if (type == "getState") {

			return BuildStateJson();

		}



		if (type == "fetchPublicIp") {

			char publicIp[NETPLAY_SESSION_HOST_LEN] = { 0 };

			bool ok = sf4e::FetchPublicIPv4(publicIp, sizeof(publicIp), 5000);

			if (!ok && m_settings.lastAdvertiseHost[0]) {

				strncpy_s(publicIp, m_settings.lastAdvertiseHost, _TRUNCATE);

				ok = true;

			}

			if (ok) {

				strncpy_s(m_settings.lastAdvertiseHost, publicIp, _TRUNCATE);

			}

			nlohmann::json r = MakeStateEnvelope();

			if (!ok) {

				nlohmann::json err;

				err["v"] = kProtocolVersion;

				err["type"] = "error";

				err["message"] = "Could not detect public IP. Enter your public or VPN address manually.";

				return err;

			}

			r["advertiseHost"] = publicIp;

			r["roomCodePreview"] = PreviewRoomCode(publicIp, m_settings.sessionPort);

			return r;

		}



		if (type == "copyText") {

			std::string text = msg.value("text", "");

			bool ok = sf4e::CopyTextToClipboardUtf8(text.c_str());

			nlohmann::json r;

			r["v"] = kProtocolVersion;

			r["type"] = ok ? "copied" : "error";

			if (!ok) {

				r["message"] = "Could not copy to clipboard.";

			}

			return r;

		}



		if (type == "previewRoomCode") {

			std::string shareIp = msg.value("advertiseHost", "");

			int port = msg.value("sessionPort", (int)m_settings.sessionPort);

			if (port < 1 || port > 65535) {

				port = m_settings.sessionPort;

			}

			nlohmann::json r = MakeStateEnvelope();

			r["roomCodePreview"] = PreviewRoomCode(shareIp.c_str(), (uint16_t)port);

			return r;

		}



		if (type == "cancel") {

			m_cancelled = true;

			m_finished = true;

			return nlohmann::json();

		}



		if (type == "start") {

			const std::string modeStr = msg.value("mode", "offline");

			int mode = (int)NetplayMode::Idle;

			if (modeStr == "host") {

				mode = (int)NetplayMode::Host;

			}

			else if (modeStr == "join") {

				mode = (int)NetplayMode::Join;

			}



			std::string name = msg.value("displayName", m_settings.displayName);

			strncpy_s(m_settings.displayName, name.c_str(), _TRUNCATE);



			int delay = msg.value("inputDelay", (int)m_settings.inputDelay);

			if (delay < 1) {

				delay = 1;

			}

			m_settings.inputDelay = (uint8_t)delay;



			int port = msg.value("sessionPort", (int)m_settings.sessionPort);

			if (port < 1 || port > 65535) {

				nlohmann::json err;

				err["v"] = kProtocolVersion;

				err["type"] = "error";

				err["message"] = "Session port must be between 1 and 65535.";

				return err;

			}

			m_settings.sessionPort = (uint16_t)port;



			memset(&m_outConfig, 0, sizeof(m_outConfig));

			m_outConfig.version = SF4E_NETPLAY_CONFIG_VERSION;

			m_outConfig.mode = mode;

			strncpy_s(m_outConfig.displayName, m_settings.displayName, _TRUNCATE);

			m_outConfig.inputDelay = m_settings.inputDelay;

			m_outConfig.sessionPort = m_settings.sessionPort;

			m_outConfig.ggpoPort = m_settings.ggpoPort;

			m_outConfig.editionSelect = m_settings.editionSelect;

			m_outConfig.roundCount = m_settings.roundCount;

			m_outConfig.roundTimeIntegral = m_settings.roundTimeIntegral;

			m_outConfig.useRelay = m_settings.useRelay;

			m_outConfig.deviceIdx = 0xff;

			m_outConfig.deviceType = 0xff;



			if (mode == (int)NetplayMode::Host) {

				strncpy_s(m_outConfig.sessionHost, m_lanIp, _TRUNCATE);

				strncpy_s(m_outConfig.roomKey, "1", _TRUNCATE);

				std::string advertise = msg.value("advertiseHost", "");

				char advBuf[NETPLAY_SESSION_HOST_LEN] = { 0 };

				strncpy_s(advBuf, advertise.c_str(), _TRUNCATE);

				sf4e::TrimRoomCodeInPlace(advBuf);

				if (advBuf[0]) {

					strncpy_s(m_settings.lastAdvertiseHost, advBuf, _TRUNCATE);

				}

			}

			else if (mode == (int)NetplayMode::Join) {

				const std::string connectMethod = msg.value("connectMethod", "direct");

				JoinRequest req;

				req.roomCode = msg.value("joinAddress", "");

				StrategyResult sr;

				if (connectMethod == "matchmaking") {

					sr = ResolveJoinMatchmaking(req);

				}

				else if (connectMethod == "autoNat") {

					sr = ResolveJoinAutoNat(req);

				}

				else {

					sr = ResolveJoinDirectIp(req);

				}

				if (!sr.ok) {

					nlohmann::json err;

					err["v"] = kProtocolVersion;

					err["type"] = "error";

					err["message"] = sr.error;

					return err;

				}

				strncpy_s(m_outConfig.sessionHost, sr.endpoint.host, _TRUNCATE);

				m_outConfig.sessionPort = sr.endpoint.port;

				FormatRoomCode(

					m_outConfig.sessionHost,

					m_outConfig.sessionPort,

					m_settings.lastJoinHost,

					sizeof(m_settings.lastJoinHost)

				);

			}



			m_finished = true;

			m_cancelled = false;

			return nlohmann::json();

		}



		nlohmann::json err;

		err["v"] = kProtocolVersion;

		err["type"] = "error";

		err["message"] = "Unknown message type.";

		return err;

	}



	bool GetLauncherUiIndexUrl(wchar_t* outUrl, int outUrlChars) {

		if (!outUrl || outUrlChars <= 0) {

			return false;

		}

		wchar_t launcherDir[MAX_PATH] = { 0 };

		wchar_t indexPath[MAX_PATH] = { 0 };

		GetModuleFileNameW(NULL, launcherDir, MAX_PATH);

		PathCchRemoveFileSpec(launcherDir, MAX_PATH);

		if (FAILED(PathCchCombine(indexPath, MAX_PATH, launcherDir, L"launcher-ui\\index.html"))) {

			return false;

		}

		wchar_t url[MAX_PATH * 2] = { 0 };

		StringCchPrintfW(url, (int)(sizeof(url) / sizeof(url[0])), L"file:///%s", indexPath);

		for (wchar_t* p = url; *p; ++p) {

			if (*p == L'\\') {

				*p = L'/';

			}

		}

		StringCchCopyW(outUrl, outUrlChars, url);

		return true;

	}



} // namespace launcher

} // namespace sf4e

