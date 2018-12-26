#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>

using std::deque;
using std::mutex;
using std::condition_variable;
using std::unique_lock;

template<typename T>
class BlockingQueue
{
public:

    explicit BlockingQueue(size_t elements = static_cast<size_t>(-1))
        : max_elements_(elements)
    {
        assert(max_elements_ > 0);
    }

    void PushFront(const T& value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            cv_queue_is_not_full_.wait(guard, [this]{ return queue_.size() < max_elements_;});
            queue_.push_front(value);
        }
        cv_queue_is_not_empty_.notify_one();
    }

    bool TryPushFront(const T& value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_full_.wait_for(guard, std::chrono::seconds(0), [this]{ return queue_.size() < max_elements_;}))
                return false;
            queue_.push_front(value);
        }
        cv_queue_is_not_empty_.notify_one();
        return true;
    }

    bool TimedPushFront(const T& value, int second)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_full_.wait_for(guard, std::chrono::seconds(second), [this]{ return queue_.size() < max_elements_;}))
                return false;
            queue_.push_front(value);
        }
        cv_queue_is_not_empty_.notify_one();
        return true;
    }

    void PushBack(const T& value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            cv_queue_is_not_full_.wait(guard, [this]{ return queue_.size() < max_elements_;});
            queue_.push_back(value);
        }
        cv_queue_is_not_empty_.notify_one();
    }

    bool TryPushBack(const T& value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_full_.wait_for(guard, std::chrono::seconds(0), [this]{ return queue_.size() < max_elements_;}))
                return false;
            queue_.push_back(value);
        }
        cv_queue_is_not_empty_.notify_one();
        return true;
    }

    bool TimedPushBack(const T& value, int second)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_full_.wait_for(guard, std::chrono::seconds(second), [this]{ return queue_.size() < max_elements_;}))
                return false;
            queue_.push_back(value);
        }
        cv_queue_is_not_empty_.notify_one();
        return true;
    }

    void PopFront(T* value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            cv_queue_is_not_empty_.wait(guard, [this]{return !queue_.empty();});
            *value = queue_.front();
            queue_.pop_front();
        }
        cv_queue_is_not_full_.notify_one();
    }

    bool TryPopFront(T* value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(0), [this]{return !queue_.empty();}))
                return false;
            *value = queue_.front();
            queue_.pop_front();
        }
        cv_queue_is_not_full_.notify_one();
        return true;
    }

    bool TimedPopFront(T* value, int second)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(second), [this]{return !queue_.empty();}))
                return false;
            *value = queue_.front();
            queue_.pop_front();
        }
        cv_queue_is_not_full_.notify_one();
        return true;
    }

    void PopBack(T* value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            cv_queue_is_not_empty_.wait(guard, [this]{return !queue_.empty();});
            *value = queue_.back();
            queue_.pop_back();
        }
        cv_queue_is_not_full_.notify_one();
    }

    bool TryPopBack(T* value)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(0), [this]{return !queue_.empty();}))
                return false;
            *value = queue_.back();
            queue_.pop_back();
        }
        cv_queue_is_not_full_.notify_one();
        return true;
    }

    bool TimedPopBack(T* value, int second)
    {
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(second), [this]{return !queue_.empty();}))
                return false;
            *value = queue_.back();
            queue_.pop_back();
        }
        cv_queue_is_not_full_.notify_one();
        return true;
    }

    void PopAll(deque<T>* values)
    {
        values->clear();
        {
            unique_lock<mutex> guard(queue_mutex_);
            cv_queue_is_not_empty_.wait(guard, [this]{return !queue_.empty();});
            values->swap(queue_);
        }
        cv_queue_is_not_full_.notify_all();
    }

    bool TryPopAll(deque<T>* values)
    {
        values->clear();
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(0), [this]{return !queue_.empty();}))
                return false;
            values->swap(queue_);
        }
        cv_queue_is_not_full_.notify_all();
        return true;
    }

    bool TimedPopAll(deque<T>* values, int second)
    {
        values->clear();
        {
            unique_lock<mutex> guard(queue_mutex_);
            if(!cv_queue_is_not_empty_.wait_for(guard, std::chrono::seconds(second), [this]{return !queue_.empty();}))
                return false;
            values->swap(queue_);
        }
        cv_queue_is_not_full_.notify_all();
        return true;
    }

    bool IsEmpty() const
    {
        unique_lock<mutex> guard(queue_mutex_);
        return queue_.empty();
    }

    bool IsFull() const
    {
        unique_lock<mutex> guard(queue_mutex_);
        return queue_.size() >= max_elements_;
    }

    void Clear()
    {
        unique_lock<mutex> guard(queue_mutex_);
        deque<T> empty_queue;
        queue_.clear();
        queue_.swap(empty_queue);
        cv_queue_is_not_full_.notify_all();
    }

private:

    BlockingQueue(const BlockingQueue&);

    const BlockingQueue& operator=(const BlockingQueue&);

private:
    size_t max_elements_;

    deque<T> queue_;

    mutex queue_mutex_;

    condition_variable cv_queue_is_not_full_;

    condition_variable cv_queue_is_not_empty_;
};
