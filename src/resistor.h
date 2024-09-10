#pragma once

#include "components.h"
#include "log.h"

class Resistor : public Component {
    double _resistance = 1; // Constant

public:
    Resistor()
        : Component{2} {}

    Resistor(const Resistor &) = delete;

    void step(Frame &frame) override {
        {
            auto voltageDrop =
                terminal(1).voltage().value() - terminal(0).voltage().value();
            dout << name() << " voltage " << voltageDrop << ", "
                 << terminal(1).voltage() << ", " << terminal(0).voltage()
                 << "\n";
            auto expectedCurrent = voltageDrop / _resistance;

            // auto [error, c] =
            //     correction(current(0), expectedCurrent, frame.learningRate);
            // frame.addError(error);
            // incCurrent(0, c);
            // dout << name() << " error " << error << "\n";
            applyExpectedCurrent(frame, expectedCurrent, 1);
            // dout << name() << " error " << error << "\n";
        }

        dout << name() << " current " << currentValue(0) << "\n";

        {
            // Correct voltage part
            auto expectedVoltageDrop = currentValue(0) * _resistance;
            // auto [error, c] =
            //     correction(voltageDrop, expectedVoltageDrop,
            //     frame.learningRate);
            // frame.addError(error);

            // terminal(0).incVoltage(-c / 2.);
            // terminal(1).incVoltage(c / 2.);

            applyExpectedVoltage(frame, expectedVoltageDrop, 1);
        }
    }

    auto &resistance(double value) {
        if (value == 0.) {
            throw std::runtime_error{"resistance cannot be zero for resistor"};
        }
        _resistance = value;
        return *this;
    }
};
