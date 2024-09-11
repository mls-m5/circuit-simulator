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

            auto expectedVoltage = _charge / _capacitance;
            applyExpectedVoltage(frame, expectedVoltage, 1 / frame.timeStep);
        }

        {
            auto d0 = terminal(1).voltage().derivative(frame.timeStep);
            auto d1 = terminal(0).voltage().derivative(frame.timeStep);
            auto dVoltageDrop = d1 - d0;
            auto expectedCurrent = _capacitance * dVoltageDrop;
            applyExpectedCurrent(frame, expectedCurrent, frame.timeStep);
            dout << name() << " dvoltage/dt " << dVoltageDrop << ", "
                 << terminal(1).voltage().derivative(frame.timeStep) << ", "
                 << terminal(0).voltage().derivative(frame.timeStep) << "\n";
        }
    }

    double charge() const {
        return current().integral(0);
    }

    Capacitor &capacitance(double value) {
        _capacitance = value;
        return *this;
    }
};
