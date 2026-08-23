#ifndef TYPES
#define TYPES

#include <float.h>
#include <algorithm>
#include <functional>
#include "Debug.hpp"

///Wrapper for float values with minimum and maximum values
struct BoundFloat
{
private:
    const float min;
    const float max;
    float value;

public:
    BoundFloat() : min(-INFINITY), max(INFINITY)
    {
        value = 0.0f;
    }
    BoundFloat(const float min, const float max, const float value) : min(min), max(max)
    {
        _assert(min < max, "Can't initialize BoundFloat with min >= max");
        this->value = std::clamp(value, min, max);
    }

    ~BoundFloat() = default;

    operator float() const
    {
        return value;
    }

    BoundFloat& operator=(const float v)
    {
        if (v < min)
            warning(std::format("Tried setting BoundFloat below min {}, set to {} instead.", v, min));
        else if (v > max)
            warning(std::format("Tried setting BoundFloat above max {}, set to {} instead.", v, max));

        value = std::clamp(v, min, max);
        return *this;
    }
};
#define PositiveFloat(v) BoundFloat(0.0f, INFINITY, v)
#define NegativeFloat(v) BoundFloat(-INFINITY, 0.0f, v)
#define UnitFloat(v) BoundFloat(0.0f, 1.0f, v)

///Function wrapper that can be called without checking if it's pointing to null
template<typename T>
struct Function;

template<typename Res, typename... ArgTypes>
struct Function<Res(ArgTypes...)>
{
private:
    std::function<Res(ArgTypes...)> func;

public:
    Function() = default;
    explicit Function(std::function<Res(ArgTypes...)> f) : func(std::move(f)) {}

    ~Function() = default;

    Function& operator=(const Function& other) = delete;
    Function& operator=(Function&& other) = delete;

    Function& operator=(const std::function<Res(ArgTypes...)>& f)
    {
        this->func = f;
        return *this;
    }

    Res operator()(ArgTypes... args) const
    {
        if (this->func)
        {
            if constexpr (std::is_void_v<Res>)
            {
                this->func(args...);
            }
            else
            {
                return this->func(args...);
            }
        }
        else if constexpr (!std::is_void_v<Res>)
        {
            return Res{};
        }
        return void();
    }
};

#endif