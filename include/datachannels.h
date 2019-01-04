#ifndef LIBDATACHANNELS_H_
#define LIBDATACHANNELS_H_
#include <stdint.h>
#include <chrono>
#include <memory>
#include <string>
#include <functional>

namespace datachannels
{

class Timer
{
public:
	using shared = std::shared_ptr<Timer>;
public:
	virtual ~Timer() = default;
	virtual void Cancel() = 0;
	virtual void Retry(const std::chrono::milliseconds& ms) = 0;
};
	
class TimeService
{
public:
	uint64_t GetNow();
	Timer::shared CreateTimer(const std::chrono::milliseconds& ms,std::function<void(void) > timeout);
};

class Datachannel
{
public:
	enum MessageType
	{
		UTF8,
		Binary
	};
	
	struct Options
	{
		
	};

	using shared = std::shared_ptr<Datachannel>;
public:
	virtual ~Datachannel() = default;
	virtual bool Send(MessageType type, const uint8_t* data, const uint64_t size)  = 0;
	virtual bool Close() = 0;
	
	// Event handlers
	virtual void onMessage(const std::function<void(MessageType, const uint8_t*,uint64_t)>& callback) = 0;
	
};

class Transport
{
public:
	virtual ~Transport() = default;
	virtual size_t ReadPacket(uint8_t *data, uint32_t size) = 0;
	virtual size_t WritePacket(uint8_t *data, uint32_t size) = 0; 
	
	virtual void OnPendingData(std::function<void(void)> callback) = 0;
};

class Endpoint
{
public:
	enum Setup
	{
		Client,
		Server
	};	
	
	struct Options
	{
		uint16_t localPort	= 5000;
		Setup setup		= Server;
	};
	
	using shared = std::shared_ptr<Endpoint>;
public:
	// Static methods
	
	// Create new datachannel endpoint
	//	options.setup : Client/Server	
	static Endpoint::shared Create(TimeService& timeService, const Options& options) ;
	
public:
	virtual ~Endpoint() = default;
	virtual bool Start(uint16_t remotePort)  = 0;
	virtual Datachannel::shared CreateDatachannel(const Datachannel::Options& options)  = 0;
	virtual bool Close()  = 0;
	
	// Getters
	virtual uint16_t   GetLocalPort() const = 0;
	virtual uint16_t   GetRemotePort() const = 0;
	virtual Setup	   GetSetup() const = 0;
	virtual Transport& GetTransport() = 0;
};

}; //namespace

#endif
