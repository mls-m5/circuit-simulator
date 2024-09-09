
#include "simulator.h"

int main(int, char **) {
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
                      << " since error was small enough\n";
            break;
        }
    }

    circuit.verify();

    probeLog.print();

    std::cout.flush();

    return 0;
}
