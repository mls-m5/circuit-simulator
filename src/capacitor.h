#pragma once

#include "components.h"
#include "log.h"

class Capacitor : public Component {
    double _capacitance = 1;
    // double _charge = 0;

public:
    Capacitor()
        : Component{2} {};

    ~Capacitor() override {}

    void step(Frame &frame) override {
        auto d0 = terminal(1).voltage().derivative(frame.timeStep);
        auto d1 = terminal(0).voltage().derivative(frame.timeStep);
        auto dVoltageDrop = d0 - d1;
        dout << name() << " dvoltage/dt " << dVoltageDrop << ", "
             << terminal(1).voltage().derivative(frame.timeStep) << ", "
             << terminal(0).voltage().derivative(frame.timeStep) << "\n";

        {
        }

        {
            // auto expectedCurrent
        }
    }

    Capacitor &capacitance(double value) {
        _capacitance = value;
        return *this;
    }
};
