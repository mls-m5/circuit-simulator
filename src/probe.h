#pragma once

#include "components.h"
#include "log.h"

class VoltageProbe : public Component {
public:
    VoltageProbe()
        : Component{1} {}

    void step(Frame &) override {
        dout << "step voltage " << name() << ": " << terminal(0).voltage()
             << "\n";

        dout << "probe ";
        if (!name().empty()) {
            dout << name() << ": ";
        }
        for (auto &t : terminals()) {
            dout << t.voltage() << " \t";
        }
    }
};
