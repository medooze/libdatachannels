#ifndef LIBDATACHANNELS_H_
#define LIBDATACHANNELS_H_
#include <stdint.h>
#include <chrono>
#include <memory>
#include <string>
#include <functional>

#ifdef ENABLE_DATACHANNEL_LOGGING
extern int Debug(const char *msg, ...);
extern int Log(const char *msg, ...);
extern int Warning(const char *msg, ...);
extern int Error(const char *msg, ...);
#else
inline int Debug(const char *msg, ...) {};
inline int Log(const char *msg, ...) {};
inline int Warning(const char *msg, ...) {};
inline int Error(const char *msg, ...) {};
#endif


namespace datachannels
{

class Timer
{
public:
	using shared = std::shared_ptr<Timer>;
public:
	virtual ~Timer() = default;
	virtual void Cancel() = 0;
	virtual void Again(const std::chrono::milliseconds& ms) = 0;
	virtual std::chrono::milliseconds GetRepeat() const = 0;
	virtual bool IsScheduled() const = 0;
};
	
class TimeService
{
public:
	virtual ~TimeService() = default;
	virtual const std::chrono::milliseconds GetNow() const = 0;
	virtual Timer::shared CreateTimer(std::function<void(std::chrono::milliseconds)> callback) = 0;
	virtual Timer::shared CreateTimer(const std::chrono::milliseconds& ms, std::function<void(std::chrono::milliseconds)> timeout) = 0;
	virtual Timer::shared CreateTimer(const std::chrono::milliseconds& ms, const std::chrono::milliseconds& repeat, std::function<void(std::chrono::milliseconds)> timeout) = 0;
	virtual void Async(const std::function<void(std::chrono::milliseconds)>& func) = 0;
};

struct Ports
{
	uint16_t localPort = 5000;
	uint16_t remotePort = 5000;
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
	virtual bool Send(MessageType type, const uint8_t* data = nullptr, const uint64_t size = 0)  = 0;
	virtual bool Close() = 0;
	
	// Event handlers
	virtual void OnMessage(const std::function<void(MessageType, const uint8_t*,uint64_t)>& callback) = 0;
	
};

class OnDataPendingListener
{
public:
	virtual ~OnDataPendingListener() = default;
	virtual void OnDataPending() = 0;
};
	
class Transport
{
public:
	virtual ~Transport() = default;
	// Read from the transport
	virtual size_t ReadPacket(uint8_t *data, uint32_t size) = 0;
	// Write to the transport
	virtual size_t WritePacket(uint8_t *data, uint32_t size) = 0; 
};

class Endpoint
{
public:
	struct Options
	{
		Ports ports;
	};
	
	using shared = std::shared_ptr<Endpoint>;
	
public:
	virtual ~Endpoint() = default;
	virtual bool Init(const Options& options, bool associate) = 0;
	virtual Datachannel::shared CreateDatachannel(const Datachannel::Options& options)  = 0;
	virtual bool Close()  = 0;
	
	// Getters
	virtual uint16_t   GetLocalPort() const = 0;
	virtual uint16_t   GetRemotePort() const = 0;
	virtual Transport& GetTransport() = 0;
};

class Sctp
{
public:
	using shared = std::shared_ptr<Sctp>;
	
	enum class Mode
	{
		Client,
		Server
	};
	
	virtual Transport& GetTransport() = 0;
	virtual bool Close() = 0;
};

}; //namespace

#endif
