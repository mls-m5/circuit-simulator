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
        , index{probeLog.registerProbe(this)} {
        // terminal(0).enabled(false);
    }

    void step(Frame &) override {
        dout << "step voltage " << name() << ": " << terminal(0).voltage()
             << "\n";
    }
};

void Log::print() {
    // for (size_t i = 0; i < _frames; ++i) {
    dout << "probe ";

    for (size_t j = 0; j < _probes.size(); ++j) {
        auto &component = _probes.at(j);
        auto name = component->name();
        if (!name.empty()) {
            dout << name << ": ";
        }
        // dout << _data.at(i * _probes.size() + j) << " \t";
        // dout << _probes.at(j)->numTerminals()
        for (auto &t : component->terminals()) {
            dout << t.voltage() << " \t";
        }
    }

    dout << "\n";
    // }
}
