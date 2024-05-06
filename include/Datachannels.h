#ifndef LIBDATACHANNELS_H_
#define LIBDATACHANNELS_H_

#include "Message.h"

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

class DataChannel
{
public:

	struct Options
	{
		
	};

	using shared = std::shared_ptr<DataChannel>;
public:
	virtual ~DataChannel() = default;
	virtual bool Send(datachannels::MessageType type, const uint8_t* data = nullptr, const uint64_t size = 0)  = 0;
	virtual bool Close() = 0;
};

class OnTransmissionPendingListener
{
public:
	virtual ~OnTransmissionPendingListener() = default;
	virtual void OnTransmissionPending() = 0;
};

class OnDatachannelCreatedListener
{
public:
	virtual ~OnDatachannelCreatedListener() = default;
	virtual void OnDatachannelCreated(const std::shared_ptr<DataChannel>& datachannel) = 0;
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
	enum class Mode
	{
		Sever,
		Client
	};
	
	struct Options
	{
		Ports ports;
		Mode mode = Mode::Sever;
	};
	
	using shared = std::shared_ptr<Endpoint>;
	
public:
	virtual ~Endpoint() = default;
	virtual bool Init(const Options& options) = 0;
	virtual DataChannel::shared CreateDataChannel(const DataChannel::Options& options)  = 0;
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
	
	virtual Transport& GetTransport() = 0;
	virtual bool Close() = 0;
};

}; //namespace

#endif
