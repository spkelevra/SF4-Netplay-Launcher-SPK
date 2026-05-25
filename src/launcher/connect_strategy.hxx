#pragma once

#include <cstdint>
#include <string>

namespace sf4e {
namespace launcher {

	struct JoinEndpoint {
		char host[64] = { 0 };
		uint16_t port = 0;
	};

	struct JoinRequest {
		std::string roomCode;
	};

	struct StrategyResult {
		bool ok = false;
		std::string error;
		JoinEndpoint endpoint;
	};

	class INetplayConnectStrategy {
	public:
		virtual ~INetplayConnectStrategy() {}
		virtual StrategyResult ResolveJoinEndpoint(const JoinRequest& request) = 0;
	};

	StrategyResult ResolveJoinDirectIp(const JoinRequest& request);
	StrategyResult ResolveJoinMatchmaking(const JoinRequest& request);
	StrategyResult ResolveJoinAutoNat(const JoinRequest& request);

} // namespace launcher
} // namespace sf4e
