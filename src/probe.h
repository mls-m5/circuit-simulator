#pragma once

#include "components.h"
#include "log.h"

class Log : public Steppable {
    std::vector<double> _data;
    static std::vector<class VoltageProbe *> _probes;

public:
    void step(Frame &) override {}

    size_t registerProbe(VoltageProbe *probe) {
        _probes.push_back(probe);
        return _probes.size() - 1;
    }

    void print();
};

inline std::vector<class VoltageProbe *> Log::_probes;

Log probeLog;

class VoltageProbe : public Component {
    size_t index = 0;

public:
    VoltageProbe()
        : Component{1}
        , index{probeLog.registerProbe(this)} {}

    void step(Frame &) override {
        dout << "step voltage " << name() << ": " << terminal(0).voltage()
             << "\n";
    }
};

void Log::print() {
    dout << "probe ";

    for (size_t j = 0; j < _probes.size(); ++j) {
        auto &component = _probes.at(j);
        auto name = component->name();
        if (!name.empty()) {
            dout << name << ": ";
        }

        for (auto &t : component->terminals()) {
            dout << t.voltage() << " \t";
        }
    }

    dout << "\n";
}
