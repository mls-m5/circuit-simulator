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
        applyExpectedVoltage(frame, _voltage);
    }
};
