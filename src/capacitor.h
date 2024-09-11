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
            // auto &current = this->current();
            // auto _charge = current.integral(frame.timeStep);

            auto expectedVoltage = charge(frame.timeStep) / _capacitance;
            applyExpectedVoltage(frame, expectedVoltage);
        }

        {
            auto d0 = terminal(1).voltage().derivative(frame.timeStep);
            auto d1 = terminal(0).voltage().derivative(frame.timeStep);
            auto dVoltageDrop = d1 - d0;
            auto expectedCurrent = _capacitance * dVoltageDrop;
            applyExpectedCurrent(frame, expectedCurrent);
            dout << name() << " dvoltage/dt " << dVoltageDrop << ", "
                 << terminal(1).voltage().derivative(frame.timeStep) << ", "
                 << terminal(0).voltage().derivative(frame.timeStep) << "\n";
            dout << " " << name() << " charge = " << charge(frame.timeStep)
                 << "\n";
        }
    }

    double charge(double timeStep) const {
        return current().integral(timeStep);
    }

    Capacitor &capacitance(double value) {
        _capacitance = value;
        return *this;
    }
};
