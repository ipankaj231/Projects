#include <iostream>
#include <vector>
#include <unordered_map>
#include <stack>
#include <sstream>
#include <cmath>
#include <stdexcept>

class VirtualMachine {
public:
    VirtualMachine() {
        // Initialize registers to 0
        for (int i = 0; i < 6; ++i) {
            registers[i] = 0;
        }
        pc = 0;
        running = false;
        memory = new int[100];  // Simulating heap memory
        stackPointer = 99;  // Top of the stack in memory
    }

    ~VirtualMachine() {
        delete[] memory;  // Clean up heap memory
    }

    // Load bytecode into memory
    void load(const std::vector<std::string>& bytecode) {
        program = bytecode;
    }

    // Run the bytecode
    void run() {
        pc = 0;  // Reset program counter
        running = true;
        
        while (running && pc < program.size()) {
            std::string instruction = program[pc++];
            try {
                execute(instruction);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                running = false;
            }
        }
    }

    // Define label and its corresponding program counter
    void defineLabel(const std::string& label) {
        labels[label] = program.size();  // Label stores the position in the program
    }

private:
    int registers[6];  // Registers R0 to R5
    int* memory;  // Heap memory (simulated with a simple array)
    int stackPointer;  // Stack pointer for function calls
    std::vector<std::string> program;  // Bytecode instructions
    int pc;  // Program counter
    bool running;  // VM running status
    std::stack<int> callStack;  // Stack for function calls
    std::unordered_map<std::string, int> labels;  // Label addresses

    // Execute a single instruction
    void execute(const std::string& instruction) {
        std::string op = instruction.substr(0, 3);  // Operation (e.g., "ADD")
        
        if (op == "MOV") {
            mov(instruction);
        } else if (op == "ADD") {
            arithmetic(instruction, "ADD");
        } else if (op == "SUB") {
            arithmetic(instruction, "SUB");
        } else if (op == "MUL") {
            arithmetic(instruction, "MUL");
        } else if (op == "DIV") {
            arithmetic(instruction, "DIV");
        } else if (op == "MOD") {
            arithmetic(instruction, "MOD");
        } else if (op == "EXP") {
            arithmetic(instruction, "EXP");
        } else if (op == "GT") {
            comparison(instruction, "GT");
        } else if (op == "LT") {
            comparison(instruction, "LT");
        } else if (op == "EQ") {
            comparison(instruction, "EQ");
        } else if (op == "PRINT") {
            print(instruction);
        } else if (op == "JMP") {
            jump(instruction);
        } else if (op == "JEQ") {
            jumpIfEqual(instruction);
        } else if (op == "CALL") {
            call(instruction);
        } else if (op == "RET") {
            ret();
        } else if (op == "ALLOC") {
            alloc(instruction);
        } else if (op == "STORE") {
            store(instruction);
        } else if (op == "LOAD") {
            loadFromHeap(instruction);
        } else {
            throw std::invalid_argument("Unknown instruction: " + instruction);
        }
    }

    void mov(const std::string& instruction) {
        int reg = instruction[4] - '0';
        int value = std::stoi(instruction.substr(6));
        registers[reg] = value;
    }

    void arithmetic(const std::string& instruction, const std::string& op) {
        int reg1 = instruction[4] - '0';
        int reg2 = instruction[6] - '0';
        
        if (op == "ADD") {
            registers[reg1] += registers[reg2];
        } else if (op == "SUB") {
            registers[reg1] -= registers[reg2];
        } else if (op == "MUL") {
            registers[reg1] *= registers[reg2];
        } else if (op == "DIV") {
            if (registers[reg2] == 0) {
                throw std::runtime_error("Error: Division by zero!");
            }
            registers[reg1] /= registers[reg2];
        } else if (op == "MOD") {
            if (registers[reg2] == 0) {
                throw std::runtime_error("Error: Modulus by zero!");
            }
            registers[reg1] %= registers[reg2];
        } else if (op == "EXP") {
            registers[reg1] = std::pow(registers[reg1], registers[reg2]);
        }
    }

    void comparison(const std::string& instruction, const std::string& op) {
        int reg1 = instruction[4] - '0';
        int reg2 = instruction[6] - '0';
        bool result = false;

        if (op == "GT") {
            result = registers[reg1] > registers[reg2];
        } else if (op == "LT") {
            result = registers[reg1] < registers[reg2];
        } else if (op == "EQ") {
            result = registers[reg1] == registers[reg2];
        }

        registers[reg1] = result ? 1 : 0;  // Store boolean result as 1 or 0
    }

    void print(const std::string& instruction) {
        int reg = instruction[6] - '0';
        std::cout << "Register " << reg << ": " << registers[reg] << std::endl;
    }

    void jump(const std::string& instruction) {
        std::string label = instruction.substr(4);
        if (labels.find(label) == labels.end()) {
            throw std::invalid_argument("Error: Undefined label " + label);
        }
        pc = labels[label];
    }

    void jumpIfEqual(const std::string& instruction) {
        int reg1 = instruction[4] - '0';
        int reg2 = instruction[6] - '0';
        std::string label = instruction.substr(8);
        
        if (registers[reg1] == registers[reg2]) {
            jump("JMP " + label);
        }
    }

    void call(const std::string& instruction) {
        std::string label = instruction.substr(5);
        callStack.push(pc);
        if (labels.find(label) == labels.end()) {
            throw std::invalid_argument("Error: Undefined label " + label);
        }
        pc = labels[label];
    }

    void ret() {
        if (callStack.empty()) {
            throw std::runtime_error("Error: Return from empty call stack!");
        }
        pc = callStack.top();
        callStack.pop();
    }

    void alloc(const std::string& instruction) {
        int size = std::stoi(instruction.substr(6));
        if (stackPointer - size < 0) {
            throw std::runtime_error("Error: Out of memory!");
        }
        stackPointer -= size;
    }

    void store(const std::string& instruction) {
        int reg = instruction[6] - '0';
        int addr = std::stoi(instruction.substr(8));
        memory[addr] = registers[reg];
    }

    void loadFromHeap(const std::string& instruction) {
        int reg = instruction[6] - '0';
        int addr = std::stoi(instruction.substr(8));
        if (addr < 0 || addr >= 100) {
            throw std::out_of_range("Error: Invalid memory address!");
        }
        registers[reg] = memory[addr];
    }
};

int main() {
    VirtualMachine vm;

    // Define bytecode with labels and heap operations
    std::vector<std::string> bytecode = {
        "MOV R0 10",   // Move 10 into R0
        "MOV R1 5",    // Move 5 into R1
        "MUL R0 R1",   // Multiply R0 and R1
        "MOD R0 R1",   // Modulo R0 by R1
        "EXP R0 R1",   // Exponentiate R0 by R1
        "STORE R0 0",  // Store R0 value at memory address 0
        "LOAD R0 0",   // Load value from memory address 0 into R0
        "PRINT R0",    // Print R0
        "JMP end",     // Jump to 'end' label
        "end:",        // Label for end
        "PRINT R0"     // Print R0 (final value)
    };

    // Load bytecode
    vm.load(bytecode);

    // Define labels after loading bytecode
    vm.defineLabel("end");

    // Run VM
    vm.run();

    return 0;
}
