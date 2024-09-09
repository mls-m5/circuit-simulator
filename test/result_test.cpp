#include "battery.h"
#include "circuit.h"
#include "components.h"
#include "ground.h"
#include "log.h"
#include "mls-unit-test/unittest.h"
#include "probe.h"
#include "resistor.h"

constexpr auto e = 0.0001;

TEST_SUIT_BEGIN(result_test)

TEST_CASE("basic resistor test") {
    dout.useBufferedOutput();
    auto circuit = Circuit{};
    const auto stepSize = .1f;
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
    runSimulation(circuit, stepSize);

    dout.flush(2000);
    probeLog.print();

    EXPECT_NEAR(probe.terminal(0).voltage(), 1.5, e);
}

TEST_CASE("double resistor test") {
    dout.useBufferedOutput();
    auto circuit = Circuit{};
    const auto stepSize = .1f;
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
    runSimulation(circuit, stepSize);

    dout.flush(2000);
    probeLog.print();

    EXPECT_NEAR(probe1.terminal(0).voltage(), 1.5 / 2, e);
    EXPECT_NEAR(probe2.terminal(0).voltage(), 1.5, e);
}

TEST_SUIT_END
