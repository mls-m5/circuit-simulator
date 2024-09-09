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
    // auto error = expection - value;
    // return error * step;
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
    virtual void step(Frame &frame) = 0;
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
        if (!t) {
            throw std::invalid_argument{"terminal is null"};
        }
        _connectedTerminals.push_back(t);
        t->node(this);
    }

    void step(Frame &frame) override {
        double sumCurrent = 0;
        for (auto c : _connectedTerminals) {
            sumCurrent += c->current();
        }

        auto error = sumCurrent / _connectedTerminals.size();

        frame.addError(error);

        // The sum current is expected to be be 0, Kirchhoffs law
        for (auto c : _connectedTerminals) {
            c->incCurrent(-error * frame.stepSize);
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

    // Current going through the component in the pointed direction
    double _stepCurrent = 0;
    std::string _name;

public:
    double current(const Terminal &terminal) const {
        auto index = std::distance(_terminals.data(), &terminal);
        return current(index);
    }
    void incCurrent(Terminal &terminal, double value) {
        auto index = std::distance(_terminals.data(), &terminal);
        return incCurrent(index, value);
    }

    std::string_view name() const {
        return _name;
    }

    void name(std::string_view value) {
        _name = value;
    }

    // virtual double current(size_t) const = 0;
    // virtual void incCurrent(size_t, double value) = 0;

    Component(const Component &) = delete;
    virtual ~Component() = default;

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

    Terminal &terminal(size_t index) {
        return _terminals.at(index);
    }

    int numTerminals() const {
        return _terminals.size();
    }

    std::span<Terminal> terminals() {
        return _terminals;
    }

    std::span<const Terminal> terminals() const {
        return _terminals;
    }

    // Return mean voltage between two terminals
    // double meanNodeVoltage() {
    //     if (_terminals.size() != 2) {
    //         throw std::runtime_error{
    //             "trying to run meanVoltage() that does not have two
    //             terminals"};
    //     }

    //     return (_terminals.front().node()->voltage() +
    //             _terminals.back().node()->voltage()) /
    //            2;
    // }

    virtual void beginFrame() = 0;

    virtual double current(size_t n) const {
        return terminalDirection(n, _stepCurrent);
    }

    virtual void incCurrent(size_t n, double value) {
        _stepCurrent += terminalDirection(n, value);
    }

    // virtual void verify() {
    //     for (auto &t: _terminals) {
    //         // TODO: This is for testing at this stage
    //         if (!t.node()) {
    //             throw std::runti
    //         }
    //     }
    // }
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
    return _parent->incCurrent(*this, v);
}

struct Ground : public Component {
    Ground()
        : Component{1} {
        name("gnd");
    }

    void step(Frame &frame) override {
        // Todo: Make ground pin the voltage of terminal directly to ground
        auto error = terminal(0).voltage();
        frame.addError(error);
        terminal(0).incVoltage(-error * frame.stepSize);
    }

    void beginFrame() override {}
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

    void beginFrame() override {
        // _stepCurrent = 0;
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

    void beginFrame() override {}

    void step(Frame &frame) override {
        auto voltageDrop = terminal(1).voltage() - terminal(0).voltage();
        {
            // Correct current part
            auto expectedCurrent = voltageDrop / _resistance;
            auto [error, c] =
                correction(current(0), expectedCurrent, frame.stepSize);
            frame.addError(error);
            incCurrent(0, c);
        }

        // std::cout << name() << " current " << current(0) << "\n";

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
        if (!value) {
            throw std::runtime_error{"resistance cannot be zero for resistor"};
        }
        _resistance = value;
        return *this;
    }
};

class Log : public Steppable {
    std::vector<double> _data;
    // size_t _frames = 0;
    static std::vector<class VoltageProbe *> _probes;

public:
    void step(Frame &) override {
        // ++_frames;
        // auto required = (_frames + 1) * _probes.size();
        // if (required > _data.size()) {
        //     _data.resize(required);
        // }
    }

    void log(size_t probeIndex, double value) {
        // _data.at(_frames * _probes.size() + probeIndex) = value;
    }

    size_t registerProbe(VoltageProbe *probe) {
        _probes.push_back(probe);
        return _probes.size() - 1;
    }

    void print();
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

    void step(Frame &frame) override {
        std::cout << "step voltage " << name() << ": " << terminal(0).voltage()
                  << "\n";
    }

    void beginFrame() override {}
};

void Log::print() {
    // for (size_t i = 0; i < _frames; ++i) {
    std::cout << "probe ";

    for (size_t j = 0; j < _probes.size(); ++j) {
        auto &component = _probes.at(j);
        auto name = component->name();
        if (!name.empty()) {
            std::cout << name << ": ";
        }
        // std::cout << _data.at(i * _probes.size() + j) << " \t";
        // std::cout << _probes.at(j)->numTerminals()
        for (auto &t : component->terminals()) {
            std::cout << t.voltage() << " \t";
        }
    }

    std::cout << "\n";
    // }
}

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

int main(int argc, char *argv[]) {
    std::cout << "hello there\n";

    constexpr auto stepSize = .1f;

    auto circuit = Circuit{};

    if (true) {
        // Possible future ascii-art syntax
        // Without the numbers
        std::cout << R"_(
      ·------·2
      |      |
      |      R1
      |      |
      B=1.5  ·1
      |      |
      |      R2
      |      |
      ·-----··0
            |
            GND
)_";

        circuit.create<Battery>({0, 2})->voltage(1.5).name("B");
        circuit.create<Resistor>({1, 2})->resistance(1).name("R1");
        circuit.create<Resistor>({0, 1})->resistance(1).name("R2");
        circuit.create<VoltageProbe>({0})->name("V0");
        circuit.create<VoltageProbe>({1})->name("V1");
        circuit.create<VoltageProbe>({2})->name("v2");
        circuit.create<Ground>({0});
    }
    else {
        // Possible future ascii-art syntax
        // Without the numbers
        std::cout << R"_(
      ·-----·1----V1
      |     |
      |     R1
      B     |
      |     |
      |     |
      ·-----·0
            |
            G
)_";

        circuit.create<Battery>({0, 1})->voltage(1.5).name("B");
        circuit.create<Resistor>({0, 1})->resistance(10).name("R1");
        // circuit.create<VoltageProbe>({0})->name("V0");
        circuit.create<VoltageProbe>({1})->name("v1");
        circuit.create<Ground>({0});
    }

    for (size_t i = 0; i < 1000; ++i) {
        auto frame = Frame{
            .stepSize = stepSize,
        };
        circuit.step(frame);

        std::cout << "error: " << frame.error << "\t on " << frame.numParameters
                  << " parameters\n";

        if (frame.error < 0.0001) {
            std::cout << "Stopped on iteration " << i
                      << " since error was small\n";
            break;
        }
    }

    circuit.verify();

    probeLog.print();

    std::cout.flush();

    return 0;
}
