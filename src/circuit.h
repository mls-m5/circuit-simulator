#pragma once

#include "components.h"
#include "probe.h"
#include <memory>

class Circuit {
    std::vector<std::unique_ptr<Component>> _components;
    std::vector<std::unique_ptr<Node>> _nodes;

public:
    Circuit() {}

    Node *node(size_t index) {
        while (index + 1 > _nodes.size()) {
            _nodes.push_back(std::make_unique<Node>());
        }
        return _nodes.at(index).get();
    }

    Component *add(std::unique_ptr<Component> component,
                   std::vector<size_t> connections) {
        auto c = component.get();
        _components.push_back(std::move(component));

        for (size_t i = 0; i < connections.size(); ++i) {
            auto &terminal = c->terminal(i);
            auto node = this->node(connections.at(i));
            node->addTerminal(&terminal);
        }
        return c;
    }

    template <typename T, typename... Args>
    T *create(Args... args, std::vector<size_t> connections) {
        add(std::make_unique<T>(args...), connections);
        return static_cast<T *>(_components.back().get());
    }

    void step(Frame &frame) {
        probeLog.step(frame);

        for (auto &n : _nodes) {
            n->step(frame);
        }

        for (auto &c : _components) {
            c->step(frame);
            dout << "Component " << c->name();
            for (auto &t : c->terminals()) {
                dout << " v, " << t.voltage() << ", ";
            }
            dout << "\n";
            dout << "  current " << c->name() << " i0 " << c->current(0);
            if (c->numTerminals() > 1) {
                dout << " i1 " << c->current(1) << "\n";
            }
        }

        for (auto &n : _nodes) {
            n->applyCorrection();
        }

        for (auto &c : _components) {
            c->applyCorrection();
        }
    }

    void verify() {
        for (auto &c : _components) {
            for (auto &t : c->terminals()) {
                if (!t.node()) {
                    throw std::runtime_error{"component " +
                                             std::string{c->name()} +
                                             " has a non-connected terminal"};
                }
            }
        }
    }
};

void runSimulation(Circuit &circuit, double stepSize) {
    for (size_t i = 0; i < 1000; ++i) {
        dout << " ----------------- step " << i << " ----------------------\n";
        auto frame = Frame{
            .learningRate = stepSize,
        };
        circuit.step(frame);

        dout << "error: " << frame.error << "\t on " << frame.numParameters
             << " parameters\n";

        if (frame.error < 0.0001) {
            dout << "Stopped on iteration " << i
                 << " since error was small enough (" << frame.error << ")\n";
            break;
        }
    }
};
