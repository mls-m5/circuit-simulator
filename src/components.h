#pragma once

#include "log.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

double learningRate = .1;

double lerp(double a, double b, double t) {
    return a * (1. - t) + b * t;
}

struct CorrectionResult {
    double error;
    double correction;
};

CorrectionResult correction(double value, double expection, double step) {
    auto res = CorrectionResult{};
    res.error = expection - value;
    res.correction = res.error * step;
    return res;
}

// How much current that goes into a component
double terminalDirection(size_t num, double value) {
    return (num == 0) ? value : -value;
}

struct Frame {
    double error = 0;
    double stepSize = .1;
    size_t numParameters = 0;

    void addError(double v) {
        error += std::abs(v);
        ++numParameters;
    }
};

class Steppable {
public:
    Steppable() = default;
    Steppable(const Steppable &) = delete;
    Steppable(Steppable &&) = delete;
    Steppable &operator=(const Steppable &) = delete;
    Steppable &operator=(Steppable &&) = delete;

    virtual void step(Frame &frame) = 0;
    virtual ~Steppable() = default;
};

// Terminal connected to a component
class Terminal {
private:
    // double _voltage = 0;

    bool _enabled = true;
    class Node *_node = nullptr;
    class Component *_parent = nullptr;
    double _direction = 1;

public:
    Terminal(Terminal &&) = delete;
    Terminal &operator=(const Terminal &) = default;
    Terminal &operator=(Terminal &&) = delete;
    Terminal(const Terminal &) = default;
    ~Terminal() = default;

    Terminal(Component *parent, double direction)
        : _parent{parent}
        , _direction{direction} {}

    // constexpr bool enabled() const {
    //     return _enabled;
    // }

    // constexpr bool enabled(bool e) {
    //     return _enabled = e;
    // }

    constexpr double voltage() const;
    constexpr void incVoltage(double v) const;

    double current() const;
    void incCurrent(double v);

    constexpr const Node *node() const {
        if (!_node) {
            throw std::runtime_error{"node not set in terminal"};
        }
        return _node;
    }

    constexpr const Node *node(Node *node) {
        return _node = node;
    }

    constexpr Node *node() {
        return _node;
    }

    constexpr Component *parent() {
        return _parent;
    }
};

// Connection between terminals
class Node : Steppable {
    std::vector<Terminal *> _connectedTerminals;
    double _voltage = 0;

public:
    Node(const Node &) = delete;
    Node() = default;
    Node(Node &&) = delete;
    Node &operator=(const Node &) = delete;
    Node &operator=(Node &&) = delete;

    ~Node() override = default;

    void addTerminal(Terminal *t) {
        if (!t) {
            throw std::invalid_argument{"terminal is null"};
        }
        _connectedTerminals.push_back(t);
        t->node(this);
    }

    void step(Frame &frame) override;

    constexpr double voltage() const {
        return _voltage;
    }

    constexpr void incVoltage(double value) {
        _voltage += value;
    }

    constexpr double voltage(double v) {
        return _voltage = v;
    }
};

class Component : public Steppable {
    std::vector<Terminal> _terminals;

    // Current going through the component in the pointed direction
    double _stepCurrent = 0;
    std::string _name;

public:
    Component(Component &&) = delete;
    Component &operator=(const Component &) = delete;
    Component &operator=(Component &&) = delete;
    Component(const Component &) = delete;

    Component(size_t numTerminals) {
        if (numTerminals == 1) {
            _terminals = {
                {this, 1},
            };
        }
        else {
            _terminals = {
                {this, 1},
                {this, -1},
            };
        }
    }

    ~Component() override = default;

    double current(const Terminal &terminal) const {
        auto index = std::distance(_terminals.data(), &terminal);
        return current(static_cast<size_t>(index));
    }
    void incCurrent(Terminal &terminal, double value) {
        auto index = std::distance(_terminals.data(), &terminal);
        return incCurrent(static_cast<size_t>(index), value);
    }

    std::string_view name() const {
        return _name;
    }

    auto &name(std::string_view value) {
        _name = value;
        return *this;
    }

    Terminal &terminal(size_t index) {
        return _terminals.at(index);
    }

    size_t numTerminals() const {
        return _terminals.size();
    }

    std::span<Terminal> terminals() {
        return _terminals;
    }

    std::span<const Terminal> terminals() const {
        return _terminals;
    }

    virtual double current(size_t n) const {
        return terminalDirection(n, _stepCurrent);
    }

    virtual void incCurrent(size_t n, double value) {
        _stepCurrent += terminalDirection(n, value);
    }
};

void Node::step(Frame &frame) {
    double sumCurrent = 0;
    for (auto c : _connectedTerminals) {
        if (c->parent()->numTerminals() == 1) {
            continue;
        }
        sumCurrent += c->current();
    }

    auto error = sumCurrent / static_cast<double>(_connectedTerminals.size());

    frame.addError(error);

    // The sum current is expected to be be 0, Kirchhoffs law
    for (auto c : _connectedTerminals) {
        if (c->parent()->numTerminals() == 1) {
            continue;
        }
        c->incCurrent(-error * frame.stepSize);
    }
}

constexpr double Terminal::voltage() const {
    return _node->voltage();
}

constexpr void Terminal::incVoltage(double v) const {
    return _node->incVoltage(v);
}

double Terminal::current() const {
    return _parent->current(*this);
}

void Terminal::incCurrent(double v) {
    return _parent->incCurrent(*this, v);
}

struct Ground : public Component {
    Ground()
        : Component{1} {
        name("gnd");
    }

    ~Ground() override = default;

    void step(Frame &frame) override {
        // Todo: Make ground pin the voltage of terminal directly to ground
        auto error = terminal(0).voltage();
        frame.addError(error);
        terminal(0).incVoltage(-error * frame.stepSize);
    }
};

class Battery : public Component {
    double _voltage = 0;

public:
    Battery()
        : Component{2} {}

    Battery &voltage(double v) {
        _voltage = v;
        return *this;
    }

    void step(Frame &frame) override {
        auto currentVoltage = terminal(1).voltage() - terminal(0).voltage();
        auto error = currentVoltage - _voltage;

        frame.addError(error);
        error /= 2;

        terminal(1).incVoltage(-error * frame.stepSize);
        terminal(0).incVoltage(error * frame.stepSize);
    }
};

class Resistor : public Component {
    double _resistance = 1; // Constant

public:
    Resistor()
        : Component{2} {}

    Resistor(const Resistor &) = delete;

    void step(Frame &frame) override {
        auto voltageDrop = terminal(1).voltage() - terminal(0).voltage();
        dout << name() << " voltage " << voltageDrop << ", "
             << terminal(1).voltage() << ", " << terminal(0).voltage() << "\n";
        {
            // Correct current part
            auto expectedCurrent = voltageDrop / _resistance;
            auto [error, c] =
                correction(current(0), expectedCurrent, frame.stepSize);
            frame.addError(error);
            incCurrent(0, c);
            dout << name() << " error " << error << "\n";
        }

        dout << name() << " current " << current(0) << "\n";

        {
            // Correct voltage part
            auto expectedVoltageDrop = current(0) * _resistance;
            auto [error, c] =
                correction(voltageDrop, expectedVoltageDrop, frame.stepSize);
            frame.addError(error);

            terminal(0).incVoltage(-c / 2.);
            terminal(1).incVoltage(c / 2.);
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
