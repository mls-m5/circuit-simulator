#include <iostream>
#include <memory>
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
    Terminal(const Terminal &) = default;
    Terminal() = default;

    constexpr bool enabled() const {
        return _enabled;
    }

    constexpr bool enabled(bool e) {
        return _enabled = e;
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
        if (!_node) {
            throw std::runtime_error{"node not set in terminal"};
        }
        return _node;
    }

    constexpr const Node *node(Node *node) {
        return _node = node;
    }
};

// Connection between terminals
class Node {
    std::vector<const Terminal *> _connections;
    double _voltage = 0;
    double _current = 0;

public:
    Node(const Node &) = delete;
    Node() = default;

    void addTerminal(Terminal *t) {
        _connections.push_back(t);
        t->node(this);
    }

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
    Component(const Component &) = delete;
    virtual ~Component() = default;

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

class Log {
    std::vector<double> _data;
    size_t _frames = 0;
    static std::vector<class Probe *> _probes;

public:
    void step() {
        ++_frames;
        auto required = (_frames + 1) * _probes.size();
        if (required > _data.size()) {
            _data.resize(required);
        }
    }

    void log(size_t probeIndex, double value) {
        _data.at(_frames * _probes.size() + probeIndex) = value;
    }

    size_t registerProbe(Probe *probe) {
        _probes.push_back(probe);
        return _probes.size() - 1;
    }

    void print() {
        for (size_t i = 0; i < _frames; ++i) {
            std::cout << "probe ";

            for (size_t j = 0; j < _probes.size(); ++j) {

                std::cout << _data.at(i * _probes.size() + j);
            }

            std::cout << "\n";
        }
    }
};

std::vector<class Probe *> Log::_probes;

Log probeLog;

class Probe : public Component {
    size_t index;

public:
    Probe()
        : Component{1} {
        terminal(0).enabled(false);
        index = probeLog.registerProbe(this);
    }

    void step() override {
        probeLog.log(index, terminal(0).node()->voltage());
    }

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
    Component *create(Args... args, std::vector<size_t> connections) {
        return add(std::make_unique<T>(args...), connections);
    }

    void step() {
        probeLog.step();

        for (auto &c : _components) {
            c->step();
        }
    }
};

int main(int argc, char *argv[]) {
    std::cout << "hello there\n";

    auto circuit = Circuit{};

    circuit.create<Probe>({0});

    circuit.step();
    circuit.step();
    circuit.step();
    circuit.step();

    probeLog.print();

    return 0;
}
