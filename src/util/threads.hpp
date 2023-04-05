#ifndef SRC_UTIL_THREADS_HPP
#define SRC_UTIL_THREADS_HPP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "concurrentqueue.h"
#pragma clang diagnostic pop

#include "util/log.hpp"
#include <concepts>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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
            requires std::is_nothrow_move_assignable_v<T>;

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
                requires std::is_nothrow_move_assignable_v<Y>;

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
                    "...!"
                );

                return output;
            }

        private:
            Receiver(std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue_)
                : queue {std::move(queue_)}
            {}

            template<class Y>
            friend std::pair<Sender<Y>, Receiver<Y>> create()
                requires std::is_nothrow_move_assignable_v<Y>;

            std::shared_ptr<moodycamel::ConcurrentQueue<T>> queue;
        }; // class Sender

        template<class T>
        std::pair<Sender<T>, Receiver<T>> create()
            requires std::is_nothrow_move_assignable_v<T>
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

        // TODO: exceptions!

        void lock(std::function<void(T&...)> func)
        {
            std::unique_lock lock {this->mutex};

            std::apply(func, this->tuple);
        }

        void lock(std::function<void(const T&...)> func) const
        {
            std::unique_lock lock {this->mutex};

            std::apply(func, this->tuple);
        }

        bool try_lock(std::function<void(T&...)> func)
        {
            std::unique_lock<std::mutex> lock {};

            if (lock.try_lock(this->mutex))
            {
                std::apply(func, this->tuple);
            }
            else
            {
                return false;
            }
        }

        bool try_lock(std::function<void(const T&...)> func) const
        {
            std::unique_lock<std::mutex> lock {};

            if (lock.try_lock(this->mutex))
            {
                std::apply(func, this->tuple);
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

} // namespace util

#endif // SRC_UTIL_THREADS_HPP
