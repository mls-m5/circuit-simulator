#pragma once

#include "log.h"
#include <functional>
#include <iterator>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

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

using Constant = double;

class Variable {
    double _previous = 0;
    double _currentValue = 0;
    double _correction = 0;
    double _integral =
        0; // Accummulated value, used for capacitors and inductors
    double _previousIntegral = 0;

    Variable *_next = nullptr;

public:
    Variable() = default;

    constexpr void next(Variable *variable) {
        _next = variable;
    }

    constexpr Variable *next() {
        return _next;
    }

    constexpr Variable(double defaultValue)
        : _previous{defaultValue}
        , _currentValue{defaultValue} {}

    constexpr void applyCorrection(double time) {
        _currentValue += _correction;
        _integral = integral(time);
        _correction = 0;
    }

    constexpr void stepTime() {
        _previous = _currentValue;
        _previousIntegral = _integral;
    }

    constexpr void operator+=(double value) {
        _correction += value;
    }

    constexpr double operator*(double value) const {
        return _currentValue * value;
    }

    constexpr double value() const {
        return _currentValue;
    }

    constexpr double derivative(double time) const {
        return (_currentValue - _previous) / time;
    }

    constexpr void incDeriviative(double time, double value) {
        _currentValue += value * time;
    }

    constexpr double integral(double time) const {
        return _previousIntegral + (_currentValue + _previous) / 2. * time;
    }

    constexpr void incIntegral(double time, double value) {
        _currentValue += value / time * 2.;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Variable &var) {
        stream << var._currentValue << "(" << var._correction << ")";
        return stream;
    }
};

struct Frame {
    double error = 0;
    double learningRate = .1;
    double timeStep = .001;
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

    constexpr const Variable &voltage() const;
    constexpr void incVoltage(double v) const;
    constexpr void incVoltageDerivative(double step, double v) const;

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

    double direction() const {
        return _direction;
    }
};

// Connection between terminals
class Node : Steppable {
    std::vector<Terminal *> _connectedTerminals;
    Variable _voltage = 0;
    double _currentFlow = 0; // Used only for statistics, not for equations

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

    // Get how much current flows through this node
    constexpr double current() {
        return _currentFlow;
    }

    constexpr Variable &voltage() {
        return _voltage;
    }

    constexpr const Variable &voltage() const {
        return _voltage;
    }

    constexpr void incVoltage(double value) {
        _voltage += value;
    }

    constexpr void incVoltageDerivative(double time, double value) {
        _voltage.incDeriviative(time, value);
    }

    // constexpr void applyCorrection() {
    //     _voltage.applyCorrection();
    // }
};

class Component : public Steppable {
    std::vector<Terminal> _terminals;

    // Current going through the component in the direction pointed by the
    // terminal direction
    Variable _current = 0;
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

    void registerVariables(std::function<void(Variable &)> f) {
        f(_current);
    }

    double current(const Terminal &terminal) const {
        auto index = std::distance(_terminals.data(), &terminal);
        return currentValue(static_cast<size_t>(index));
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

    const Terminal &terminal(size_t index) const {
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

    Variable &current() {
        return _current;
    }

    const Variable &current() const {
        return _current;
    }

    virtual double currentValue(size_t n) const {
        return _current * terminal(n).direction();
    }

    virtual void incCurrent(size_t n, double value) {
        _current += value * terminal(n).direction();
    }

    // virtual void applyCorrection() {
    //     _current.applyCorrection();
    // }

    // multiplier should be timeStep for derivatives and 1/timestep for
    // integrals
    void applyExpectedVoltage(Frame &frame, double expectedVoltage) {
        auto currentVoltage =
            terminal(1).voltage().value() - terminal(0).voltage().value();

        auto [error, c] =
            correction(currentVoltage, expectedVoltage, frame.learningRate);
        frame.addError(error);

        c /= 2.;

        terminal(0).incVoltage(-c);
        terminal(1).incVoltage(c);
        dout << name() << ": when applying V: expected=" << expectedVoltage
             << " error=" << error << " correction= " << c << "\n";
    }

    // multiplier should be timeStep for derivatives and 1/timestep for
    // integrals
    void applyExpectedCurrent(Frame &frame, double expectedCurrent) {
        auto [error, c] =
            correction(currentValue(0), expectedCurrent, frame.learningRate);
        frame.addError(error);
        incCurrent(0, c);
        dout << name() << ": when applying I: expected=" << expectedCurrent
             << " error=" << error << " correction= " << c << "\n";
    }
};

void Node::step(Frame &frame) {
    double sumCurrent = 0;
    size_t numTerminals = 0;
    _currentFlow = 0;
    for (auto c : _connectedTerminals) {
        if (c->parent()->numTerminals() == 1) {
            continue;
        }
        auto current = c->current();
        sumCurrent += current;
        ++numTerminals;
        if (current > 0) {
            _currentFlow += current;
        }
    }

    auto error = sumCurrent / static_cast<double>(numTerminals);

    frame.addError(error);

    // The sum current is expected to be be 0: Kirchhoffs law
    for (auto c : _connectedTerminals) {
        if (c->parent()->numTerminals() == 1) {
            continue;
        }
        c->incCurrent(-error * frame.learningRate);
    }
}

constexpr const Variable &Terminal::voltage() const {
    return _node->voltage();
}

constexpr void Terminal::incVoltage(double v) const {
    return _node->incVoltage(v);
}

constexpr inline void Terminal::incVoltageDerivative(double time,
                                                     double v) const {
    _node->incVoltageDerivative(time, v);
}

double Terminal::current() const {
    return _parent->current(*this);
}

void Terminal::incCurrent(double v) {
    return _parent->incCurrent(*this, v);
}
