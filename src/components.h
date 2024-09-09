#pragma once

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

public:
    Variable() = default;

    Variable(double defaultValue)
        : _previous{defaultValue}
        , _currentValue{defaultValue} {}

    constexpr void applyCorrection() {
        _previous = _currentValue;
        _currentValue += _correction;
        _correction = 0;
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

    // double derivative(double stepSize) const {
    //     // TODO: Look at this, I dont think this is how I should do it
    //     return (_currentValue + _correction - _previous);
    // }

    friend std::ostream &operator<<(std::ostream &stream, const Variable &var) {
        stream << var._currentValue << "(" << var._correction << ")";
        return stream;
    }
};

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

    constexpr const Variable &voltage() const {
        return _voltage;
    }

    constexpr void incVoltage(double value) {
        _voltage += value;
    }

    constexpr void applyCorrection() {
        _voltage.applyCorrection();
    }
};

class Component : public Steppable {
    std::vector<Terminal> _terminals;

    // Current going through the component in the pointed direction
    Variable _stepCurrent = 0;
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

    virtual double current(size_t n) const {
        return _stepCurrent * terminal(n).direction();
    }

    virtual void incCurrent(size_t n, double value) {
        _stepCurrent += value * terminal(n).direction();
        // _stepCurrent += terminalDirection(n, value);
    }

    virtual void applyCorrection() {
        _stepCurrent.applyCorrection();
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

constexpr const Variable &Terminal::voltage() const {
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
