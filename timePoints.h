#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <stdexcept>

// This class is used to print message at particular points in the code with the time elapsed since the last call
// shamelessly taken from my other github project microProjects/memoryAccessPattern
class TimePoints
{
public:
    TimePoints() {
        m_checkPoint = m_clock.now();
    }

    // Reset the timer to "now"
    void reset()
    {
        m_checkPoint = m_clock.now();
    }

    // Print the time elapsed since the last call to reset() or checkPoint(). The message must contains the string "{TIME}"
    // That will be replaced by the time in milliseconds.
    // Return the time elapsed in ms
    long long checkPoint(const char* message)
    {
        std::string strMsg = std::string(message);
        return checkPoint(strMsg);
    }

    // Print the time elapsed since the last call to reset() or checkPoint(). The message must contains the string "{TIME}"
    // That will be replaced by the time in milliseconds
    // Return the time elapsed in ms
    long long checkPoint(std::string& message)
    {
        std::string placeholder = "{TIME}";
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_checkPoint).count();
        auto position = message.find(placeholder);
        if (position == message.npos)
            throw std::invalid_argument("message should contain {TIME}");

        std::cout << message.replace(position, placeholder.size(), std::to_string(duration)) << std::endl;
        reset();
        return duration;
    }

protected:
    std::chrono::steady_clock m_clock;
    std::chrono::steady_clock::time_point m_checkPoint;
};

