#pragma once

#include "components.h"
#include "log.h"

class Capacitor : public Component {
    double _capacitance = 1;

public:
    Capacitor()
        : Component{2} {};

    ~Capacitor() override {}

    void step(Frame &frame) override {
        {
            auto &current = this->current();
            auto _charge = current.integral(frame.timeStep);

#todo figure out this

            auto expectedVoltage = ;
        }

        {
            auto d0 = terminal(1).voltage().derivative(frame.timeStep);
            auto d1 = terminal(0).voltage().derivative(frame.timeStep);
            auto dVoltageDrop = d0 - d1;
            auto expectedCurrent = _capacitance * dVoltageDrop;
            applyExpectedCurrent(frame, expectedCurrent, 1 / frame.timeStep);
            dout << name() << " dvoltage/dt " << dVoltageDrop << ", "
                 << terminal(1).voltage().derivative(frame.timeStep) << ", "
                 << terminal(0).voltage().derivative(frame.timeStep) << "\n";
        }
    }

    Capacitor &capacitance(double value) {
        _capacitance = value;
        return *this;
    }
};
