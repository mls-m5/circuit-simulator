#pragma once

#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

struct CustomStream : public std::ostream {
    std::fstream dummyOutput;
    std::stringstream bufferedOutput;

    CustomStream()
        : std::ostream{nullptr} {
        rdbuf(std::cout.rdbuf());
    }

    ~CustomStream() {
        // flush();
    }

    void enable(bool status) {
        if (status) {
            rdbuf(std::cout.rdbuf());
        }
        else {
            rdbuf(dummyOutput.rdbuf());
        }
    }

    void useBufferedOutput() {
        bufferedOutput.clear();
        rdbuf(bufferedOutput.rdbuf());
    }

    void flush(size_t maxLength = static_cast<size_t>(-1)) {
        auto str = bufferedOutput.str();
        if (maxLength > str.size()) {
            std::cout << str << std::flush;
        }
        else {
            std::cout.write(str.data(), maxLength / 2);
            size_t start = str.size() - maxLength / 2;
            std::cout << "...\n\n\n...";
            std::cout.write(str.data() + start, str.size() - 1 - start);
        }

        bufferedOutput.clear();

        enable(true); // Redirect to stdout from here
    }
};

inline auto dout = CustomStream{};
