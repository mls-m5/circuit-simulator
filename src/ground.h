#pragma once

#include "components.h"

struct Ground : public Component {
    Ground()
        : Component{1} {
        name("gnd");
    }

    ~Ground() override = default;

    void step(Frame &frame) override {
        // Todo: Make ground pin the voltage of terminal directly to ground
        auto error = terminal(0).voltage();
        frame.addError(error);
        terminal(0).incVoltage(-error * frame.stepSize);
    }
};
