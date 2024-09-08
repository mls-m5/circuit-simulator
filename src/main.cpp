#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

double learningRate = .1;

double lerp(double a, double b, double t) {
    return a * (1. - t) + b * t;
}

double correction(double value, double expection, double step) {
    auto error = expection - value;
    return error * step;
}

class Steppable {
    virtual void step(double stepSize) = 0;
};

// Terminal connected to a component
class Terminal {
private:
    double _voltage;

    bool _enabled = true;
    class Node *_node = nullptr;
    class Component *_parent = nullptr;
    double _direction = 1;

public:
    Terminal(const Terminal &) = default;
    Terminal(Component *parent, double direction)
        : _parent{parent}
        , _direction{direction} {}

    constexpr bool enabled() const {
        return _enabled;
    }

    constexpr bool enabled(bool e) {
        return _enabled = e;
    }

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
};

// Connection between terminals
class Node : Steppable {
    std::vector<Terminal *> _connectedTerminals;
    double _voltage = 0;

public:
    Node(const Node &) = delete;
    Node() = default;
    virtual ~Node() = default;

    void addTerminal(Terminal *t) {
        _connectedTerminals.push_back(t);
        t->node(this);
    }

    void step(double stepSize) override {
        double sumCurrent = 0;
        for (auto c : _connectedTerminals) {
            sumCurrent += c->current();
        }

        auto correction = sumCurrent / _connectedTerminals.size();

        // The sum current is expected to be be 0, Kirchhoffs law
        for (auto c : _connectedTerminals) {
            c->incCurrent(-correction * stepSize);
        }
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
};

class Component : public Steppable {
    std::vector<Terminal> _terminals;

public:
    double current(const Terminal &terminal) const {
        auto index = std::distance(_terminals.data(), &terminal);
        return current(index);
    }
    void incCurrent(Terminal &terminal) {
        auto index = std::distance(_terminals.data(), &terminal);
        return incCurrent(index);
    }

    virtual double current(size_t) const = 0;
    virtual void incCurrent(size_t) = 0;

    Component(const Component &) = delete;
    virtual ~Component() = default;

    Component(size_t numTerminals) {
        if (numTerminals == 0) {
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

    virtual void beginFrame() = 0;
};

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
    return _parent->incCurrent(*this);
}

struct Ground : public Component {
    Ground()
        : Component{1} {}

    void step(double stepSize) override {
        // Todo: Make ground pin the voltage of terminal directly to ground
        terminal(0).incVoltage(-terminal(1).voltage() * stepSize);
    }

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
    double _resistance = 1; // Constant

    double _stepCurrent = 0;

public:
    Resistor()
        : Component{2} {}

    Resistor(const Resistor &) = delete;

    void beginFrame() override {}

    void step(double stepSize) override {
        auto voltageDrop = terminal(1).voltage() - terminal(0).voltage();
        {
            // Correct current part
            auto expectedCurrent = voltageDrop / _resistance;
            auto c = correction(_stepCurrent, expectedCurrent, stepSize);
            _stepCurrent += c;
        }

        {
            // Correct voltage part
            auto expectedVoltageDrop = _stepCurrent * _resistance;
            auto c = correction(voltageDrop, expectedVoltageDrop, stepSize);

            terminal(0).incVoltage(-c / 2.);
            terminal(1).incVoltage(c / 2.);
        }
    }

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
    size_t index = 0;

public:
    VoltageProbe()
        : Component{1}
        , index{probeLog.registerProbe(this)} {
        terminal(0).enabled(false);
    }

    // void stepCurrent() override {}

    // void stepVoltage() override {
    //     probeLog.log(index, terminal(0).node()->voltage());
    // }

    void step(double stepSize) override {}

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
    std::cout << R"_(
      ·0----·1--P1
      |     |
      |     R1
      B     ·2--P2
      |     R2
      |     |
      ·-----·3--P3
            |
            G
)_";

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
