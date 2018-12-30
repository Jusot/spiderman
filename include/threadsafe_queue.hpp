#pragma once


/*
 * threadsafe_queue<T> tq;
 * T val;
 * while (true)
 * {
 *     tq.wait_pop(val);
 *     ....;
 * }
 */
template <typename T>
class threadsafe_queue
{
private:
    mutable ::std::mutex mut;
    ::std::condition_variable cond;
    ::std::queue<T> _queue;

public:
    threadsafe_queue() = default;
    threadsafe_queue(const threadsafe_queue&) = delete;

    void push(T t)
    {
        ::std::lock_guard<::std::mutex> lg(mut);
        _queue.push(t);
        cond.notify_one();
    }

    void wait_pop(T &t)
    {
        ::std::unique_lock<::std::mutex> ul(mut);
        cond.wait(ul, [this] { return !_queue.empty(); });
        t = _queue.front();
        _queue.pop();
    }

    ::std::shared_ptr<T> wait_pop()
    {
        ::std::unique_lock<::std::mutex> ul(mut);
        cond.wait(ul, [this] { return !_queue.empty(); });
        auto res = ::std::make_shared<T>(_queue.front());
        _queue.pop();
        return res;
    }

    bool try_pop(T &t)
    {
        ::std::lock_guard<::std::mutex> lg(mut);
        if (_queue.empty()) return false;
        t = _queue.front();
        _queue.pop();
        return true;
    }

    ::std::shared_ptr<T> try_pop()
    {
        ::std::lock_guard<::std::mutex> lg(mut);
        if (_queue.empty()) return ::std::make_shared<T>(nullptr);
        auto res = ::std::make_shared<T>(_queue.front());
        _queue.pop();
        return res;
    }

    bool is_empty()
    {
        ::std::lock_guard<::std::mutex> lg(mut);
        return _queue.empty();
    }
};