#ifndef FAKETIMESERVICE_H
#define FAKETIMESERVICE_H

#include "Datachannels.h"

using namespace std::chrono_literals;

class FakeTimeService : public datachannels::TimeService
{
public:
	class TimerImpl : public datachannels::Timer, public std::enable_shared_from_this<TimerImpl>
	{
	public:
			using shared = std::shared_ptr<TimerImpl>;
	public:
		TimerImpl(FakeTimeService& timeService, const std::chrono::milliseconds& repeat, std::function<void(std::chrono::milliseconds) > callback) :
			timeService(timeService),
			next(0),
			repeat(repeat),
			callback(callback)
		{
		}
			
		virtual ~TimerImpl() 
		{
		}
		
		TimerImpl(const TimerImpl&) = delete;
		
		virtual void Cancel() override
		{
			//We don't have to repeat this
			repeat = 0ms;

			//If not scheduled
			if (!next.count())
				//Nothing
				return;

			//Get all timers at that tick
			auto result = timeService.timers.equal_range(next);

			//Reset next tick
			next = 0ms;

			// Iterate over the range
			for (auto it = result.first; it != result.second; ++it)
			{
				//If it is the same impl
				if (it->second.get()==this)
				{
					//Remove this one
					timeService.timers.erase(it);
					//Found
					break;
				}
			}
		}
		
		virtual void Again(const std::chrono::milliseconds& ms) override
		{
			//Get next run
			auto next = timeService.GetNow() + ms;

			//Remove us
			Cancel();

			//Set next tick
			this->next = next;

			//Add to timer list
			timeService.timers.insert({next, this->shared_from_this()});
		}
		
		virtual std::chrono::milliseconds GetRepeat() const override { return repeat; };
		
		FakeTimeService&  timeService;
		std::chrono::milliseconds next;
		std::chrono::milliseconds repeat;
		std::function<void(std::chrono::milliseconds)> callback;
	};
public:	
	void SetNow(const std::chrono::milliseconds& ms)  
	{
		//Ensure now is after now
		if (ms<now) return;
		//Store new now
		now = ms;
		//Timers triggered
		std::vector<TimerImpl::shared> triggered;
		//Get all timers to process in this loop
		for (auto it = timers.begin(); it!=timers.end(); )
		{
			//Check it we are still on the time
			if (it->first>now)
				//It is yet to come
				break;
			//Get timer
			triggered.push_back(it->second);
			//Remove from the list
			it = timers.erase(it);
		}
		
		//Now process all timers triggered
		for (auto& timer : triggered)
		{
			//UltraDebug("-EventLoop::Run() | timer triggered at ll%u\n",now.count());
			//We are executing
			timer->next = 0ms;
			//Execute it
			timer->callback(now);
			//If we have to reschedule it again
			if (timer->repeat.count() && !timer->next.count())
			{
				//Set next
				timer->next = now + timer->repeat;
				//Schedule
				timers.insert({timer->next, timer});
			}
		}
		
	}
	
	virtual const std::chrono::milliseconds GetNow() const override
	{ 
		return now;
	};
	
	virtual datachannels::Timer::shared CreateTimer(std::function<void(std::chrono::milliseconds)> callback) override
	{
		//Create timer without scheduling it
		auto timer = std::make_shared<TimerImpl>(*this,0ms,callback);
		//Done
		return std::static_pointer_cast<datachannels::Timer>(timer);
	};
	
	virtual datachannels::Timer::shared CreateTimer(const std::chrono::milliseconds& ms, std::function<void(std::chrono::milliseconds)> timeout) override
	{
		//Timer without repeat
		return CreateTimer(ms,0ms,timeout);
	};
	
	virtual datachannels::Timer::shared CreateTimer(const std::chrono::milliseconds& ms, const std::chrono::milliseconds& repeat, std::function<void(std::chrono::milliseconds)> timeout) override
	{
		//Create timer
		auto timer = std::make_shared<TimerImpl>(*this,repeat,timeout);

		//Set next tick
		timer->next = GetNow() + ms;;

		//Add to timer list
		timers.insert({timer->next, timer});
		
		//Done
		return std::static_pointer_cast<datachannels::Timer>(timer);
	};

private:
	std::multimap<std::chrono::milliseconds,TimerImpl::shared> timers;
	std::chrono::milliseconds now;
};

#endif /* FAKETIMESERVICE_H */

