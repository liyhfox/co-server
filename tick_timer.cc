#include "tick_timer.h"


#include <chrono>
#include <atomic>
#include <thread>

using namespace std::chrono;
using std::lock_guard;
using std::unique_lock;
using std::atomic;
using std::thread;

TickTimer::TickTimer()
{
    thread work_thread(&TickTimer::PressButton, this);
    work_thread.detach();
}

TickTimer::~TickTimer()
{

}

uint64_t TickTimer::AddTimer(uint64_t interval, const CallbackFunction& callback)
{
    return SetTimer(interval, false, callback);
}

uint64_t TickTimer::AddPeriodTimer(uint64_t interval, const CallbackFunction& callback)
{
    return SetTimer(interval, true, callback);
}

bool TickTimer::ModifyTimer(uint64_t timer_id, uint64_t interval)
{
    lock_guard<mutex> locker(timer_mutex_);

    auto it = timer_map_.find(timer_id);
    if(it != timer_map_.end())
    {
        Timer& timer = it->second;
        timer.interval = interval;
        timer.revision++;
        SetTicker(timer_id, interval, timer.revision);
        return true;
    }

    return false;
}

bool TickTimer::RemoveTimer(uint64_t timer_id)
{
    lock_guard<mutex> locker(timer_mutex_);
    auto it = timer_map_.find(timer_id);
    if(it != timer_map_.end())
    {
        timer_map_.erase(it);
        return true;
    }
    return false;
}

bool TickTimer::EnableTimer(uint64_t timer_id)
{
    lock_guard<mutex> locker(timer_mutex_);
    auto it = timer_map_.find(timer_id);
    if(it != timer_map_.end())
    {
        if(!it->second.is_enable)
        {
            it->second.is_enable = true;
            it->second.revision++;
            SetTicker(timer_id, it->second.interval, it->second.revision);
        }
        return true;
    }
    return false;
}

bool TickTimer::DisableTimer(uint64_t timer_id)
{
    lock_guard<mutex> locker(timer_mutex_);
    auto it = timer_map_.find(timer_id);
    if(it != timer_map_.end())
    {
        it->second.is_enable = false;
        return true;
    }
    return false;
}

void TickTimer::Clear()
{
    lock_guard<mutex> ticker_locker(ticker_mutex_);
    while(!ticker_queue_.empty())
        ticker_queue_.pop();

    lock_guard<mutex> locker(timer_mutex_);
    timer_map_.clear();
}

void TickTimer::PressButton()
{
    while(true)
    {
        uint64_t left_time = 0;
        if(GetLatestTickerLeftTime(&left_time))
        {
            unique_lock<mutex> locker(ticker_mutex_);
            cv_new_ticker_time_.wait_for(locker, milliseconds(left_time));
        }
        else
        {
            unique_lock<mutex> locker(ticker_mutex_);
            cv_new_ticker_time_.wait(locker, [this]{ return !ticker_queue_.empty(); });
        }
        //
        Alarm();
    }
}

void TickTimer::Alarm()
{
    uint64_t timer_id;
    Timer timer;
    while(GetAlarmingTicker(&timer_id, &timer))
    {
        if(timer.callback)
            timer.callback();

        if(timer.is_period)
        {
            lock_guard<mutex> timer_locker(timer_mutex_);
            auto it = timer_map_.find(timer_id);
            if(it != timer_map_.end())
            {
                SetTicker(timer_id, timer.interval, timer.revision);
            }
        }
    }
}

uint64_t TickTimer::SetTimer(uint64_t interval, bool is_period, const CallbackFunction& callback)
{
    lock_guard<mutex> locker(timer_mutex_);

    uint64_t id = NewTimerId();
    Timer& timer = timer_map_[id];
    timer.interval = interval;
    timer.is_enable = true;
    timer.is_period = is_period;
    timer.callback = callback;

    SetTicker(id, interval, 0);
    return id;
}

void TickTimer::SetTicker(uint64_t timer_id, uint64_t interval, uint8_t revision)
{
    uint64_t prev_top_time;
    if(GetLatestTickerTime(&prev_top_time))
    {
        DoSetTicker(timer_id, interval, revision);
        uint64_t new_top_time;
        if(GetLatestTickerTime(&new_top_time) && new_top_time < prev_top_time)
        {
            cv_new_ticker_time_.notify_one();
        }
    }
    else
    {
        DoSetTicker(timer_id, interval, revision);
        cv_new_ticker_time_.notify_one();
    }
}

void TickTimer::DoSetTicker(uint64_t timer_id, uint64_t interval, uint8_t revision)
{
    lock_guard<mutex> locker(ticker_mutex_);

    Ticker ticker = { GetCurrentTime() + interval, timer_id, revision};
    ticker_queue_.push(ticker);
}

bool TickTimer::GetLatestTickerTime(uint64_t* time)
{
    lock_guard<mutex> locker(ticker_mutex_);

    if(ticker_queue_.empty())
        return false;

    *time = ticker_queue_.top().time;
    return true;
}

bool TickTimer::GetLatestTickerLeftTime(uint64_t* time)
{
    if(GetLatestTickerTime(time))
    {
        uint64_t now = GetCurrentTime();
        if(*time < now)
            *time = 0;
        else
            *time -= now;

        return true;
    }

    return false;
}

bool TickTimer::GetAlarmingTicker(uint64_t* timer_id, Timer* timer)
{
    while(true)
    {
        lock_guard<mutex> ticker_locker(ticker_mutex_);

        if(ticker_queue_.empty() || ticker_queue_.top().time > GetCurrentTime())
            return false;

        Ticker ticker = ticker_queue_.top();
        ticker_queue_.pop();

        lock_guard<mutex> timer_locker(timer_mutex_);
        auto it = timer_map_.find(ticker.timer_id);
        if(it == timer_map_.end())
            continue;

        // ignore disabeld ticker
        if(!it->second.is_enable)
            continue;

        // ignore outdated ticker
        if(it->second.revision != ticker.revision)
            continue;

        *timer_id = ticker.timer_id;
        *timer = it->second;

        if(!timer->is_period)
            timer_map_.erase(it);

        break;
    }

    return true;
}

uint64_t TickTimer::GetCurrentTime() const
{
    auto tp = time_point_cast<milliseconds>(high_resolution_clock::now());
    return tp.time_since_epoch().count();
}

uint64_t TickTimer::NewTimerId()
{
    static volatile atomic<uint64_t> timer_id(0);
    return timer_id++;
}
