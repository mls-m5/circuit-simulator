#pragma once

#include "components.h"

class Battery : public Component {
    double _voltage = 0; // Constant

public:
    Battery()
        : Component{2} {}

    Battery &voltage(double v) {
        _voltage = v;
        return *this;
    }

    void step(Frame &frame) override {
        applyExpectedVoltage(frame, _voltage, 1);
        // auto currentVoltage =
        //     terminal(1).voltage().value() - terminal(0).voltage().value();

        // auto error = currentVoltage - _voltage;

        // frame.addError(error);
        // error /= 2;

        // terminal(1).incVoltage(-error * frame.learningRate);
        // terminal(0).incVoltage(error * frame.learningRate);
    }
};
