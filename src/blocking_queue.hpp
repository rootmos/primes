#ifndef blocking_queue_hpp
#define blocking_queue_hpp

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

#include "container_traits.hpp"

template<typename T, typename Q = std::deque<T>>
class blocking_queue
{
public:
    typedef std::function<bool (const T&)> Predicate;

private:
    // Our private data

    Q queue;

    std::mutex mutex;
    std::condition_variable flag;

    std::atomic_bool running;


    // Adapters for obtaining the first element

    template<typename S>
    typename std::enable_if<container_traits::has_front<S>::value, const T&>::type
    front_adapter () const  { return queue.front(); };

    template<typename S>
    typename std::enable_if<container_traits::has_top<S>::value, const T&>::type
    front_adapter () const { return queue.top (); };


    // Adapters for poping the first element

    template<typename S>
    typename std::enable_if<container_traits::has_pop<S>::value, void>::type
    pop_adapter () { queue.pop (); };

    template<typename S>
    typename std::enable_if<container_traits::has_pop_front<S>::value, void>::type
    pop_adapter () { queue.pop_front (); };


    // Adapters for pushing an element

    template<typename S>
    typename std::enable_if<container_traits::has_push<S>::value, void>::type
    push_adapter (const T& t) { queue.push (t); };

    template<typename S>
    typename std::enable_if<container_traits::has_push_back<S>::value, void>::type
    push_adapter (const T& t) { queue.push_back (t); };


    // Our two versions for the predicate used while waiting

    bool pop_or_not ()
    {
        return !running || !queue.empty();
    };

    bool pop_or_not_with_predicate (Predicate& p)
    {
        return !running || ( !queue.empty() && p(front_adapter<Q> ()) );
    };


    // The common method for poping an element

    bool pop (T& t, std::function<bool()> pred)
    {
        std::unique_lock<std::mutex> lock (mutex);

        flag.wait (lock, pred);

        if (!running)
            return false;

        t = front_adapter<Q> ();
        pop_adapter<Q> ();
        return true;
    };



public:

    // Our constructor

    blocking_queue ():
        running (true)
    {};


    // The method used for pushing an element into the queue

    void push (const T& t)
    {
        std::unique_lock<std::mutex> lock (mutex);

        push_adapter<Q>(t);
        flag.notify_one ();
    };


    // The two methods for poping an element

    bool pop (T& t)
    {
        return pop (t, std::bind (&blocking_queue::pop_or_not, this));
    };

    // Here the user may provide a predicate to be waited for while popping

    bool pop (T& t, Predicate& p)
    {
        return pop (t, std::bind (&blocking_queue::pop_or_not_with_predicate,
                                    this, std::ref(p)));
    };


    // Simple method for checking whether the queue is empty or not

    bool empty ()
    {
        return queue.empty ();
    };


    // The method for stopping the queue

    void stop ()
    {
        std::unique_lock<std::mutex> lock (mutex);
        running = false;

        flag.notify_all ();
    };
};

#endif
