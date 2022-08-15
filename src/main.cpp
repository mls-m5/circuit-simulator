#include <iostream>
#include <vector>

double learningRate = .1;

double lerp(double a, double b, double t) {
    return a * (1. - t) + b * t;
}

// Terminal connected to a component
class Terminal {
    double _voltage;
    double _current;

    bool _enabled = true;
    const struct Node *_node = nullptr;

public:
    constexpr bool enabled() const {
        return _enabled;
    }

    constexpr double voltage() const {
        return _voltage;
    }

    constexpr double voltage(double v) {
        return _voltage = v;
    }

    constexpr double current() const {
        return _current;
    }

    constexpr double current(double c) {
        return _current = c;
    }

    constexpr const Node *node() const {
        return _node;
    }
};

// Connection between terminals
class Node {
    std::vector<const Terminal *> _connections;
    double _voltage = 0;
    double _current = 0;

public:
    void step() {
        size_t sum = 0;
        double v = 0;
        for (auto c : _connections) {
            if (c->enabled()) {
                ++sum;
                v += c->voltage();
                _current += c->current();
            }
        }
        _voltage = v / sum;
    }

    constexpr double voltage() const {
        return _voltage;
    }

    /// Residual current, should always be near zero when circuit is balanced at
    /// the end of each simulation step
    constexpr double current() const {
        return _current;
    }
};

class Component {
    std::vector<Terminal> _terminals;

public:
    Component(size_t numTerminals) {
        _terminals.resize(numTerminals);
    }

    Terminal &terminal(size_t index) {
        return _terminals.at(index);
    }

    virtual void step() = 0;

    virtual void beginFrame() = 0;
};

struct Ground : public Component {
    Ground()
        : Component{1} {}

    void step() override {
        terminal(0).voltage(0);
    }

    void beginFrame() override {}
};

struct Battery : public Component {
    double _stepCurrent = 0;

    Battery()
        : Component{2} {}

    void beginFrame() override {
        _stepCurrent = 0;
    }

    void step() override {
        double _current = 0;

        _current =
            terminal(0).node()->current() - terminal(1).node()->current();

        _stepCurrent = lerp(_current, _stepCurrent, learningRate);

        terminal(0).current(-_stepCurrent);
        terminal(1).current(_stepCurrent);
    }
};

int main(int argc, char *argv[]) {
    std::cout << "hello there\n";
    return 0;
}
