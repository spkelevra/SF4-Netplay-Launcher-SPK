#include "connect_strategy.hxx"

#include "netplay_room_code.hxx"

namespace sf4e {
namespace launcher {

	StrategyResult ResolveJoinDirectIp(const JoinRequest& request) {
		StrategyResult r;
		if (request.roomCode.empty()) {
			r.error = "Host address is required.";
			return r;
		}
		uint16_t port = 0;
		if (!ParseRoomCode(request.roomCode.c_str(), r.endpoint.host, sizeof(r.endpoint.host), &port)) {
			r.error = "Invalid host address. Use IP:port or hostname:port (example 203.0.113.42:23456).";
			return r;
		}
		r.endpoint.port = port;
		r.ok = true;
		return r;
	}

	StrategyResult ResolveJoinMatchmaking(const JoinRequest& request) {
		(void)request;
		StrategyResult r;
		r.error = "Matchmaking is not available yet.";
		return r;
	}

	StrategyResult ResolveJoinAutoNat(const JoinRequest& request) {
		(void)request;
		StrategyResult r;
		r.error = "Automatic NAT traversal is not available yet. Use direct IP and port-forward.";
		return r;
	}

} // namespace launcher
} // namespace sf4e
