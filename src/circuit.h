#pragma once

#include "components.h"
#include "log.h"
#include <memory>

class Circuit {
    std::vector<std::unique_ptr<Component>> _components;
    std::vector<std::unique_ptr<Node>> _nodes;
    Variable *_variables = nullptr;

public:
    Circuit() {}

    Node *node(size_t index) {
        while (index + 1 > _nodes.size()) {
            _nodes.push_back(std::make_unique<Node>());
            add(_nodes.back()->voltage());
        }
        return _nodes.at(index).get();
    }

    Component *add(std::unique_ptr<Component> component,
                   std::vector<size_t> connections) {
        auto c = component.get();
        _components.push_back(std::move(component));
        _components.back()->registerVariables([this](Variable &v) { add(v); });

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

    void add(Variable &variable) {
        variable.next(_variables);
        _variables = &variable;
    }

    void step(Frame &frame) {
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
            dout << "  current " << c->name() << " i0 " << c->currentValue(0)
                 << "\n";
            if (c->numTerminals() > 1) {
                dout << " i1 " << c->currentValue(1) << "\n";
            }
        }

        for (auto variable = _variables; variable != nullptr;
             variable = variable->next()) {
            variable->applyCorrection(frame.timeStep);
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

    void stepTime() {
        for (auto variable = _variables; variable != nullptr;
             variable = variable->next()) {
            variable->stepTime();
        }
    }
};

void runSimulation(Circuit &circuit, double timeStep, double learningRate) {
    for (int t = 0; t < 100; ++t) {
        dout << " ================= time step " << t << " = " << t * timeStep
             << "s ====================\n";
        for (size_t i = 0; i < 1000; ++i) {
            dout << " ----------------- step " << i
                 << " ----------------------\n";
            auto frame = Frame{
                .learningRate = learningRate,
                .timeStep = timeStep,
            };
            circuit.step(frame);

            dout << "error: " << frame.error << "\t on " << frame.numParameters
                 << " parameters\n";

            if (frame.error < 0.0001) {
                dout << "Stopped on iteration " << i
                     << " since error was small enough (" << frame.error
                     << ")\n";
                break;
            }
        }
        circuit.stepTime();
    }
};
