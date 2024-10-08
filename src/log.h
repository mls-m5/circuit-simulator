#pragma once

#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

struct CustomStream : public std::ostream {
    std::fstream dummyOutput;
    std::stringstream bufferedOutput;

    bool isEnabled = true;

    CustomStream(bool enabledByDefault = true)
        : std::ostream{nullptr} {
        enable(enabledByDefault);
    }

    CustomStream(const CustomStream &) = delete;
    CustomStream(CustomStream &&) = delete;
    CustomStream &operator=(const CustomStream &) = delete;
    CustomStream &operator=(CustomStream &&) = delete;

    ~CustomStream() override {}

    void enable(bool status) {
        isEnabled = status;
        if (status) {
            rdbuf(std::cout.rdbuf());
        }
        else {
            rdbuf(dummyOutput.rdbuf());
        }
    }

    void useBufferedOutput() {
        isEnabled = true;
        bufferedOutput.clear();
        rdbuf(bufferedOutput.rdbuf());
    }

    void flush(size_t maxLength = static_cast<size_t>(-1)) {
        isEnabled = false;
        auto str = bufferedOutput.str();
        if (maxLength > str.size()) {
            std::cout << str << std::flush;
        }
        else {
            std::cout.write(str.data(), maxLength / 2);
            size_t start = str.size() - maxLength / 2;
            std::cout << "...\n\n\n...";
            std::cout.write(str.data() + start, str.size() - start);
        }

        bufferedOutput = {};

        enable(true); // Redirect to stdout from here
    }
};

inline auto doutInstance = CustomStream{false};

// Prevent extra work from being done
#define dout                                                                   \
    if (doutInstance.isEnabled)                                                \
    doutInstance
