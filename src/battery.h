#pragma once

#include "components.h"

class Battery : public Component {
    double _voltage = 0;

public:
    Battery()
        : Component{2} {}

    Battery &voltage(double v) {
        _voltage = v;
        return *this;
    }

    void step(Frame &frame) override {
        auto currentVoltage = terminal(1).voltage() - terminal(0).voltage();
        auto error = currentVoltage - _voltage;

        frame.addError(error);
        error /= 2;

        terminal(1).incVoltage(-error * frame.stepSize);
        terminal(0).incVoltage(error * frame.stepSize);
    }
};
