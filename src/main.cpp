#include <iostream>
#include <memory>
#include <vector>

double learningRate = .1;

double lerp(double a, double b, double t) {
    return a * (1. - t) + b * t;
}

class Steppable {
    virtual void step(double stepSize) = 0;
};

// class ComponentWithCurrent {
//     virtual double current(class Terminal& terminal) {

//     }
// };

// Terminal connected to a component
class Terminal {
    double _voltage;
    // double _current;

    bool _enabled = true;
    struct Node *_node = nullptr;
    double _direction = 1;

public:
    Terminal(const Terminal &) = default;
    Terminal(double direction)
        : _direction{direction} {}

    constexpr bool enabled() const {
        return _enabled;
    }

    constexpr bool enabled(bool e) {
        return _enabled = e;
    }

    constexpr double voltage() const;

    // constexpr double current() const {
    //     return _current;
    // }

    // constexpr double current(double c) {
    //     return _current = c;
    // }

    constexpr const Node *node() const {
        if (!_node) {
            throw std::runtime_error{"node not set in terminal"};
        }
        return _node;
    }

    constexpr void incVoltage(double v) const;

    constexpr const Node *node(Node *node) {
        return _node = node;
    }

    constexpr Node *node() {
        return _node;
    }
};

// Connection between terminals
class Node : Steppable {
    std::vector<const Terminal *> _connectedTerminals;
    double _voltage = 0;
    // double _currentError = 0;

public:
    Node(const Node &) = delete;
    Node() = default;

    void addTerminal(Terminal *t) {
        _connectedTerminals.push_back(t);
        t->node(this);
    }

    void step(double stepSize) override {
        for (auto c : _connectedTerminals) {
        }

        // size_t sum = 0;
        // double v = 0;
        // for (auto c : _connections) {
        //     if (c->enabled()) {
        //         ++sum;
        //         v += c->voltage();
        //         _currentError += c->current();
        //     }
        // }
        // _voltage = v / sum;
    }

    constexpr double voltage() const {
        return _voltage;
    }

    constexpr void incVoltage(double value) {
        _voltage += value;
    }

    constexpr double voltage(double v) {
        return _voltage = v;
    }

    /// Residual currentError, should always be near zero when circuit is
    /// balanced at the end of each simulation step
    // constexpr double currentError() const {
    //     return _currentError;
    // }
};

constexpr double Terminal::voltage() const {
    return _node->voltage();
}

constexpr void Terminal::incVoltage(double v) const {
    return _node->incVoltage(v * _direction);
}

class Component : public Steppable {
    std::vector<Terminal> _terminals;

public:
    Component(const Component &) = delete;
    virtual ~Component() = default;

    Component(size_t numTerminals) {
        if (numTerminals == 0) {
            _terminals = {
                {1},
            };
        }
        else {
            _terminals = {
                {1},
                {-1},
            };
        }
    }

    Terminal &terminal(size_t index) {
        return _terminals.at(index);
    }

    // Return mean voltage between two terminals
    double meanNodeVoltage() {
        if (_terminals.size() != 2) {
            throw std::runtime_error{
                "trying to run meanVoltage() that does not have two terminals"};
        }

        return (_terminals.front().node()->voltage() +
                _terminals.back().node()->voltage()) /
               2;
    }

    // virtual void step(double stepSize) = 0;
    // virtual void stepCurrent() = 0;
    // virtual void stepVoltage() = 0;

    virtual void beginFrame() = 0;
};

struct Ground : public Component {
    Ground()
        : Component{1} {}

    // void stepCurrent() override {
    //     // Todo: Implement
    // }

    // void stepVoltage() override {
    //     terminal(0).voltage(0);
    // }

    void step(double stepSize) {}

    void beginFrame() override {}
};

class Battery : public Component {
    double _stepCurrent = 0;
    double _voltage = 0;

public:
    Battery()
        : Component{2} {}

    double voltage(double v) {
        return _voltage = v;
    }

    void beginFrame() override {
        _stepCurrent = 0;
    }

    void step(double stepSize) override {
        auto currentVoltage = terminal(1).voltage() - terminal(0).voltage();
        auto error = currentVoltage - _voltage;

        // TODO: Check sign
        terminal(0).incVoltage(-error);
        terminal(1).incVoltage(error);
    }

    // void stepVoltage() override {
    //     double meanVoltage =
    //         (terminal(0).node()->voltage() + terminal(1).node()->voltage()) /
    //         2;

    //     terminal(0).voltage(meanVoltage + _voltage / 2.);
    //     terminal(1).voltage(meanVoltage - _voltage / 2.);
    // }

    // void stepCurrent() override {
    //     double _current = 0;

    //     _current = terminal(0).node()->currentError() -
    //                terminal(1).node()->currentError();

    //     _stepCurrent = lerp(_current, _stepCurrent, learningRate);

    //     terminal(0).current(-_stepCurrent);
    //     terminal(1).current(_stepCurrent);
    // }
};

class Resistor : public Component {
    double _resistance = 1;

    double _stepCurrent = 0;

public:
    Resistor()
        : Component{2} {}

    Resistor(const Resistor &) = delete;

    void beginFrame() override {}

    // void stepVoltage() override {
    //     auto mv = meanNodeVoltage();
    //     terminal(0).voltage(mv - _stepCurrent * _resistance);
    //     terminal(1).voltage(mv + _stepCurrent * _resistance);
    // }

    // void stepCurrent() override {
    //     auto voltageDrop =
    //         terminal(1).node()->voltage() - terminal(0).node()->voltage();
    //     auto current = voltageDrop / _resistance;
    //     _stepCurrent = lerp(current, _stepCurrent, learningRate);
    //     terminal(0).current(_stepCurrent);
    //     terminal(1).current(-_stepCurrent);
    // }

    double resistance(double value) {
        if (!value) {
            throw std::runtime_error{"resistance cannot be zero for resistor"};
        }
        return _resistance = value;
    }
};

class Log : public Steppable {
    std::vector<double> _data;
    size_t _frames = 0;
    static std::vector<class VoltageProbe *> _probes;

public:
    void step(double stepSize) override {
        ++_frames;
        auto required = (_frames + 1) * _probes.size();
        if (required > _data.size()) {
            _data.resize(required);
        }
    }

    void log(size_t probeIndex, double value) {
        _data.at(_frames * _probes.size() + probeIndex) = value;
    }

    size_t registerProbe(VoltageProbe *probe) {
        _probes.push_back(probe);
        return _probes.size() - 1;
    }

    void print() {
        for (size_t i = 0; i < _frames; ++i) {
            std::cout << "probe ";

            for (size_t j = 0; j < _probes.size(); ++j) {

                std::cout << _data.at(i * _probes.size() + j) << " ";
            }

            std::cout << "\n";
        }
    }
};

std::vector<class VoltageProbe *> Log::_probes;

Log probeLog;

class VoltageProbe : public Component {
    size_t index;

public:
    VoltageProbe()
        : Component{1} {
        terminal(0).enabled(false);
        index = probeLog.registerProbe(this);
    }

    // void stepCurrent() override {}

    // void stepVoltage() override {
    //     probeLog.log(index, terminal(0).node()->voltage());
    // }

    void beginFrame() override {}
};

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

    void step(double stepSize) {
        probeLog.step(stepSize);

        for (auto &n : _nodes) {
            n->step(stepSize);
        }
    }
};

int main(int argc, char *argv[]) {
    std::cout << "hello there\n";

    auto circuit = Circuit{};

    // Possible future ascii-art syntax
    // Without the numbers
    //  ·0----·1--P1
    //  |     |
    //  |     R1
    //  B     ·2--P2
    //  |     R2
    //  |     |
    //  ·-----·3--P3
    //        |
    //        G

    circuit.create<Battery>({0, 3})->voltage(1.5);
    circuit.create<Resistor>({1, 2})->resistance(1000);
    circuit.create<Resistor>({2, 3})->resistance(1000);
    circuit.create<VoltageProbe>({1});
    circuit.create<VoltageProbe>({2});
    circuit.create<Ground>({3});

    auto stepSize = 0.001;

    for (size_t i = 0; i < 100; ++i) {
        circuit.step(stepSize);
    }

    probeLog.print();

    std::cout.flush();

    return 0;
}
