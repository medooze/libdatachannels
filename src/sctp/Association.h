#ifndef SCTP_ASSOCIATION_H_
#define SCTP_ASSOCIATION_H_
#include <list>

#include "Datachannels.h"
#include "sctp/PacketHeader.h"
#include "sctp/Stream.h"
#include "BufferWritter.h"
#include "BufferReader.h"

using namespace std::chrono_literals;

namespace sctp
{
	
class Association : public datachannels::Transport
{
public:
	enum State
	{
		Closed,
		CookieWait,
		CookieEchoed,
		Established,
		ShutdownPending,
		ShutDownSent,
		ShutDownReceived,
		ShutDown,
		ShutDownAckSent
	};
public:
	Association(datachannels::TimeService& timeService);
	
	bool Associate();
	bool Shutdown();
	bool Abort();

	void SetLocalPort(uint16_t port) 	{ localPort = port;	}
	void SetRemotePort(uint16_t port) 	{ remotePort = port;	}
	uint16_t GetLocalPort() const 		{ return localPort;	}
	uint16_t GetRemotePort() const		{ return remotePort;	}
	State GetState() const			{ return state;		}
	bool HasPendingData() const		{ return pendingData;	}
	
	  
	virtual size_t ReadPacket(uint8_t *data, uint32_t size) override;
	virtual size_t WritePacket(uint8_t *data, uint32_t size) override;
	
	inline size_t ReadPacket(Buffer& buffer)
	{
		size_t len = ReadPacket(buffer.GetData(),buffer.GetCapacity());
		buffer.SetSize(len);
		return len;
	}
	inline size_t WritePacket(Buffer& buffer)
	{
		return WritePacket(buffer.GetData(),buffer.GetSize());
	}
	
	virtual void OnPendingData(std::function<void(void)> callback) override
	{
		onPendingData = callback;
	}
	
	static constexpr const size_t MaxInitRetransmits = 10;
	static constexpr const std::chrono::milliseconds InitRetransmitTimeout = 100ms;
private:
	void Process(const Chunk::shared& chunk);
	void SetState(State state);
	void Enqueue(const Chunk::shared& chunk);
private:
	State state = State::Closed;
	std::list<Chunk::shared> queue;
	datachannels::TimeService& timeService;
	
	uint16_t localPort = 0;
	uint16_t remotePort = 0;
	uint32_t localAdvertisedReceiverWindowCredit = 0xFFFFFFFF;
	uint32_t remoteAdvertisedReceiverWindowCredit = 0;
	uint32_t localVerificationTag = 0;
	uint32_t remoteVerificationTag = 0;
	uint32_t initRetransmissions = 0;
	datachannels::Timer::shared initTimer;
	datachannels::Timer::shared cookieEchoTimer;
	
	bool pendingData = false;
	std::function<void(void)> onPendingData;
	
	std::map<uint16_t,Stream::shared> streams;
};

}
#endif
