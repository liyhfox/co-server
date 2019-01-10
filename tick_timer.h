#ifndef TICK_TIMER_H_
#define TICK_TIMER_H_

#include <map>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

using std::map;
using std::vector;
using std::priority_queue;
using std::function;
using std::mutex;
using std::condition_variable;

class TickTimer
{
public:
    TickTimer();
    ~TickTimer();

public:
    typedef function<void ()> CallbackFunction;

    uint64_t AddTimer(uint64_t interval, const CallbackFunction& callback);

    uint64_t AddPeriodTimer(uint64_t interval, const CallbackFunction& callback);

    bool ModifyTimer(uint64_t timer_id, uint64_t interval);

    bool RemoveTimer(uint64_t timer_id);

    bool EnableTimer(uint64_t timer_id);

    bool DisableTimer(uint64_t timer_id);

    void Clear();

private:
    TickTimer(const TickTimer&) = delete;

    const TickTimer& operator= (const TickTimer&) = delete;

private:
    struct Timer
    {
        uint64_t interval;
        bool is_enable;
        bool is_period;
        uint8_t revision;
        CallbackFunction callback;
    };

    struct Ticker
    {
        uint64_t time;
        uint64_t timer_id;
        uint8_t revision; //timer revision, old revisions will be discarded
        bool operator > (const Ticker& rsh) const
        {
            return time > rsh.time;
        };
    };

private:
    void PressButton();

    void Alarm();

    uint64_t SetTimer(uint64_t interval, bool is_period, const CallbackFunction& callback);

    void SetTicker(uint64_t timer_id, uint64_t interval, uint8_t revision);

    void DoSetTicker(uint64_t timer_id, uint64_t interval, uint8_t revision);

    bool GetLatestTickerTime(uint64_t* time);

    bool GetLatestTickerLeftTime(uint64_t* time);

    bool GetAlarmingTicker(uint64_t* timer_id, Timer* timer);

    uint64_t GetCurrentTime() const;

    uint64_t NewTimerId();

private:

    mutex timer_mutex_;

    map<uint64_t, Timer> timer_map_;

    mutex ticker_mutex_;

    priority_queue<Ticker, vector<Ticker>, std::greater<Ticker> > ticker_queue_;

    condition_variable cv_new_ticker_time_;
};


#endif
