#include "battery.h"
#include "capacitor.h"
#include "circuit.h"
#include "components.h"
#include "ground.h"
#include "log.h"
#include "mls-unit-test/unittest.h"
#include "probe.h"
#include "resistor.h"

constexpr auto e = 0.0001;

constexpr auto learningRate = .1f;
constexpr auto timeStep = .001f;

TEST_SUIT_BEGIN(result_test)

TEST_CASE("basic resistor test") {
    doutInstance.useBufferedOutput();
    auto circuit = Circuit{};
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
    auto &probe = circuit.create<VoltageProbe>({1})->name("v1");
    circuit.create<Ground>({0});

    circuit.verify();
    runSimulation(circuit, timeStep, learningRate);

    dout.flush(2000);
    // probeLog.print();

    EXPECT_NEAR(probe.terminal(0).voltage().value(), 1.5, e);
}

TEST_CASE("double resistor test") {
    doutInstance.useBufferedOutput();
    auto circuit = Circuit{};
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
    circuit.create<Resistor>({1, 2})->resistance(10).name("R1");
    circuit.create<Resistor>({0, 1})->resistance(10).name("R2");
    circuit.create<VoltageProbe>({0})->name("V0");
    auto &probe1 = circuit.create<VoltageProbe>({1})->name("V1");
    auto &probe2 = circuit.create<VoltageProbe>({2})->name("v2");
    circuit.create<Ground>({0});

    circuit.verify();
    runSimulation(circuit, timeStep, learningRate);

    dout.flush(2000);
    // probeLog.print();

    EXPECT_NEAR(probe1.terminal(0).voltage().value(), 1.5 / 2, e);
    EXPECT_NEAR(probe2.terminal(0).voltage().value(), 1.5, e);
}

TEST_CASE("basic capacitor") {
    doutInstance.useBufferedOutput();
    auto circuit = Circuit{};
    // Possible future ascii-art syntax
    // Without the numbers
    std::cout << R"_(
      ·------·1-V1
      |      |
      |      |
      |      |
      B=1.5  C1
      |      |
      |      |
      |      |
      ·-----··0
            |
            GND
)_";

    circuit.create<Battery>({0, 1})->voltage(1.5).name("B");
    auto &capacitor = circuit.create<Capacitor>({0, 1})->capacitance(10);
    capacitor.name("C");
    auto &probe0 = circuit.create<VoltageProbe>({0})->name("V0");
    auto &probe1 = circuit.create<VoltageProbe>({1})->name("V1");
    circuit.create<Ground>({0});

    circuit.verify();
    runSimulation(circuit, timeStep, learningRate);

    dout.flush(3000);

    EXPECT_NEAR(probe0.terminal(0).voltage().value(), 0., e);
    EXPECT_NEAR(probe1.terminal(0).voltage().value(), 1.5, e);
    EXPECT_NEAR(circuit.node(0)->current(), 0, e);
    std::cout << capacitor.charge(timeStep) << std::endl;
}

TEST_SUIT_END
