#pragma once
#include "state_info.h"

#include <array>
#include <cstddef>

constexpr std::size_t MAX_STATES = 128;

class StateBuffer
{
private:
    std::array<StateInfo, MAX_STATES> states{};
    std::size_t currentIndex = 0;

public:
    StateInfo& current() noexcept
    {
        return states[currentIndex];
    }
    StateInfo& next() noexcept
    {
        currentIndex++;
        return states[currentIndex];
    }
    StateInfo& previous() noexcept
    {
        currentIndex--;
        return states[currentIndex];
    }

    void reset() noexcept
    {
        currentIndex = 0;
    }

    std::size_t getCurrentIndex() const noexcept
    {
        return currentIndex;
    }
};