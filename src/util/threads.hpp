#ifndef SRC_UTIL_THREADS_HPP
#define SRC_UTIL_THREADS_HPP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "blockingconcurrentqueue.h"
#pragma clang diagnostic pop

#include "util/log.hpp"
#include <concepts>
#include <condition_variable>
#include <functional>
#include <latch>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

namespace util
{
    namespace mpmc
    {
        template<class T>
        class Sender;

        template<class T>
        class Receiver;

        template<class T>
        std::pair<Sender<T>, Receiver<T>> create()
            requires std::is_move_assignable_v<T>;

        template<class T>
        class Sender
        {
        public:

            ~Sender() = default;

            Sender(const Sender&)             = default;
            Sender(Sender&&)                  = default;
            Sender& operator= (const Sender&) = default;
            Sender& operator= (Sender&&)      = default;

            void send(T&& t) const
            {
                if (!this->queue->enqueue(std::forward<T>(t)))
                {
                    throw std::bad_alloc {};
                }
            }

        private:
            Sender(std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue_)
                : queue {std::move(queue_)}
            {}

            template<class Y>
            friend std::pair<Sender<Y>, Receiver<Y>> create()
                requires std::is_move_assignable_v<Y>;

            std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue;
        }; // class Sender

        template<class T>
        class Receiver
        {
        public:

            ~Receiver() = default;

            Receiver(const Receiver&)             = default;
            Receiver(Receiver&&)                  = default;
            Receiver& operator= (const Receiver&) = default;
            Receiver& operator= (Receiver&&)      = default;

            std::optional<T> receive() const
            {
                std::optional<T> output;

                util::assertFatal(
                    this->queue->try_dequeue(output) == output.has_value(),
                    "...!");

                return output;
            }

        private:
            Receiver(std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue_)
                : queue {std::move(queue_)}
            {}

            template<class Y>
            friend std::pair<Sender<Y>, Receiver<Y>> create()
                requires std::is_move_assignable_v<Y>;

            std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue;
        }; // class Sender

        template<class T>
        std::pair<Sender<T>, Receiver<T>> create()
            requires std::is_move_assignable_v<T>
        {
            auto queue = std::make_shared<moodycamel::ConcurrentQueue<T>>();

            return {Sender<T> {queue}, Receiver<T> {std::move(queue)}};
        }
    } // namespace mpmc

    template<class... T>
    class Mutex
    {
    public:

        Mutex(T... t)
            : tuple {std::forward<T>(t)...}
        {}
        ~Mutex() = default;

        Mutex(const Mutex&)             = delete;
        Mutex(Mutex&&)                  = default;
        Mutex& operator= (const Mutex&) = delete;
        Mutex& operator= (Mutex&&)      = default;

        void lock(std::invocable<T&...> auto func) noexcept(noexcept(func))
        {
            std::unique_lock lock {this->mutex};

            std::apply(func, this->tuple);
        }

        void lock(std::invocable<const T&...> auto func) const
            noexcept(noexcept(func))
        {
            std::unique_lock lock {this->mutex};

            std::apply(func, this->tuple);
        }

        bool try_lock(std::invocable<T&...> auto func) noexcept(noexcept(func))
        {
            std::unique_lock<std::mutex> lock {this->mutex, std::defer_lock};

            if (lock.try_lock())
            {
                std::apply(func, this->tuple);
                return true;
            }
            else
            {
                return false;
            }
        }

        bool try_lock(std::invocable<const T&...> auto func) const
            noexcept(noexcept(func))
        {
            std::unique_lock<std::mutex> lock {this->mutex, std::defer_lock};

            if (lock.try_lock())
            {
                std::apply(func, this->tuple);
                return true;
            }
            else
            {
                return false;
            }
        }

    private:
        mutable std::mutex mutex;
        std::tuple<T...>   tuple;
    }; // class Mutex

    template<class T>
        requires (
            std::is_move_constructible_v<T> && std::is_move_assignable_v<T>
            || std::same_as<T, void>)
    class Future
    {
    public:

        Future()
            : completed {new std::latch {1}}
        {}
        ~Future() = default;

        Future(const Future&)             = delete;
        Future(Future&&)                  = default;
        Future& operator= (const Future&) = delete;
        Future& operator= (Future&&)      = default;

        T await()
        {
            this->completed->wait();

            return std::move(*this->try_await());
        }

        std::optional<T> try_await()
        {
            if (this->completed->try_wait())
            {
                return std::move(value);
            }

            return std::nullopt;
        }

    private:
        template<class R, class F>
        friend std::shared_ptr<Future<R>> runAsynchronously(F fn)
            requires std::is_invocable_r_v<R, F>;

        void fulfillFuture(T&& t)
        {
            this->value = std::make_optional<T>(std::move(t));

            this->completed->count_down();
        }

        std::unique_ptr<std::latch> completed;
        std::optional<T>            value;
    };

    template<>
    class Future<void>
    {
    public:
        Future()
            : completed {new std::latch {1}}
        {}
        ~Future() = default;

        Future(const Future&)             = delete;
        Future(Future&&)                  = delete;
        Future& operator= (const Future&) = delete;
        Future& operator= (Future&&)      = delete;

        void await()
        {
            this->completed->wait();
        }

        bool try_await()
        {
            return this->completed->try_wait();
        }

    private:
        template<class R, class F>
        friend std::shared_ptr<Future<R>> runAsynchronously(F fn)
            requires std::is_invocable_r_v<R, F>;

        void fulfillFuture()
        {
            this->completed->count_down();
        }

        std::unique_ptr<std::latch> completed;
    };

    class AsynchronousThreadPool
    {
    public:

        AsynchronousThreadPool()
        {
            this->workers.resize(std::thread::hardware_concurrency());

            util::assertFatal(
                this->workers.size() != 0 || this->workers.size() != ~0,
                "Invalid std::thread::hardware_concurrency() | Received value "
                "of {}",
                this->workers.size());

            for (auto& [thread, shouldStop] : this->workers)
            {
                shouldStop = std::make_unique<std::atomic<bool>>();
                shouldStop->store(false);

                thread = std::thread {
                    [&]
                    {
                        while (!shouldStop->load())
                        {
                            std::optional<std::function<void()>> maybeFunc {};

                            if (this->queue.wait_dequeue_timed(
                                    maybeFunc, 25000))
                            {
                                maybeFunc->operator() ();
                                maybeFunc = std::nullopt;
                            }
                        }
                    }};
            }
        }
        ~AsynchronousThreadPool()
        {
            for (auto& [thread, shouldStop] : this->workers)
            {
                shouldStop->store(true);
            }

            std::optional<std::function<void()>> maybeFunc {};

            for (std::size_t i = 0; i < 100; ++i) // good enough
            {
                while (queue.try_dequeue(maybeFunc))
                {
                    maybeFunc->operator() ();
                }
            }

            for (auto& [thread, shouldStop] : this->workers)
            {
                thread.join();
            }
        }

        AsynchronousThreadPool(const AsynchronousThreadPool&) = delete;
        AsynchronousThreadPool(AsynchronousThreadPool&&)      = default;
        AsynchronousThreadPool&
        operator= (const AsynchronousThreadPool&)                    = delete;
        AsynchronousThreadPool& operator= (AsynchronousThreadPool&&) = default;

        void addJob(std::function<void()>&& func)
        {
            if (!this->queue.enqueue(std::move(func)))
            {
                throw std::bad_alloc {};
            }
        }

    private:
        moodycamel::BlockingConcurrentQueue<std::function<void()>> queue;
        std::vector<std::pair<std::thread, std::unique_ptr<std::atomic<bool>>>>
            workers;
    };

    namespace
    {
        AsynchronousThreadPool& getThreadPool()
        {
            static AsynchronousThreadPool threadPool {};
            return threadPool;
        }
    } // namespace

    template<class R, class F>
    std::shared_ptr<Future<R>> runAsynchronously(F fn)
        requires std::is_invocable_r_v<R, F>
    {
        std::shared_ptr<Future<R>> future = std::make_shared<Future<R>>();

        if constexpr (std::same_as<R, void>)
        {
            getThreadPool().addJob(
                [=]
                {
                    fn();
                    future->fulfillFuture();
                });
        }
        else
        {
            getThreadPool().addJob(
                [=]
                {
                    future->fulfillFuture(fn());
                });
        }

        return future;
    }

} // namespace util

#endif // SRC_UTIL_THREADS_HPP
