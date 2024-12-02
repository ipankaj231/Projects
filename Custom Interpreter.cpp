#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <cctype>


class Interpreter {
    struct Function {
        std::vector<std::string> parameters;
        std::string body;
    };

    std::string input;
    size_t pos;
    char current_char;
    std::map<std::string, int> variables;
    std::map<std::string, Function> functions;
    std::map<std::string, std::vector<int>> arrays;

    void error(const std::string& msg) {
        throw std::runtime_error("Error: " + msg);
    }

    void advance() {
        pos++;
        if (pos < input.length())
            current_char = input[pos];
        else
            current_char = '\0';  // End of input
    }

    void skip_whitespace() {
        while (current_char != '\0' && isspace(current_char)) {
            advance();
        }
    }

    int integer() {
        std::string result;
        while (current_char != '\0' && isdigit(current_char)) {
            result += current_char;
            advance();
        }
        return std::stoi(result);
    }

    std::string identifier() {
        std::string result;
        while (current_char != '\0' && (isalnum(current_char) || current_char == '_')) {
            result += current_char;
            advance();
        }
        return result;
    }

    int factor() {
        skip_whitespace();
        if (isdigit(current_char)) {
            return integer();
        } else if (isalpha(current_char)) {
            std::string var_name = identifier();
            if (current_char == '[') {  // Array access
                advance();
                int index = expr();
                if (current_char != ']') error("Expected ']' for array access");
                advance();
                if (arrays.count(var_name)) {
                    if (index < 0 || index >= arrays[var_name].size()) error("Array index out of bounds");
                    return arrays[var_name][index];
                } else {
                    error("Undefined array: " + var_name);
                }
            } else if (variables.count(var_name)) {  // Variable access
                return variables[var_name];
            } else if (functions.count(var_name)) {  // Function call
                return call_function(var_name);
            } else {
                error("Undefined variable or function: " + var_name);
            }
        } else if (current_char == '(') {
            advance();
            int result = expr();
            if (current_char != ')') error("Expected ')'");
            advance();
            return result;
        } else {
            error("Invalid factor");
        }
        return 0;  // Should never reach here
    }

    int term() {
        int result = factor();
        while (current_char == '*' || current_char == '/') {
            char op = current_char;
            advance();
            if (op == '*')
                result *= factor();
            else
                result /= factor();
        }
        return result;
    }

    int expr() {
        int result = term();
        while (current_char == '+' || current_char == '-') {
            char op = current_char;
            advance();
            if (op == '+')
                result += term();
            else
                result -= term();
        }
        return result;
    }

    void block() {
        skip_whitespace();
        if (current_char != '{') error("Expected '{' to start block");
        advance();
        while (current_char != '\0' && current_char != '}') {
            statement();
            skip_whitespace();
            if (current_char == ';') advance();
        }
        if (current_char != '}') error("Expected '}' to end block");
        advance();
    }

    int call_function(const std::string& func_name) {
        if (!functions.count(func_name)) error("Undefined function: " + func_name);

        Function func = functions[func_name];
        skip_whitespace();

        if (current_char != '(') error("Expected '(' for function call");
        advance();

        std::vector<int> args;
        for (const auto& param : func.parameters) {
            if (args.size() > 0) {
                if (current_char != ',') error("Expected ',' between function arguments");
                advance();
            }
            args.push_back(expr());
        }

        if (current_char != ')') error("Expected ')' after function arguments");
        advance();

        std::map<std::string, int> saved_variables = variables;
        for (size_t i = 0; i < func.parameters.size(); i++) {
            variables[func.parameters[i]] = args[i];
        }

        size_t saved_pos = pos;
        std::string saved_input = input;

        input = func.body;
        pos = 0;
        current_char = input[pos];

        int result = 0;
        try {
            result = expr();
        } catch (...) {
            input = saved_input;
            pos = saved_pos;
            current_char = input[pos];
            variables = saved_variables;
            throw;
        }

        input = saved_input;
        pos = saved_pos;
        current_char = input[pos];
        variables = saved_variables;

        return result;
    }

    void statement() {
        skip_whitespace();
        if (current_char == '\0') return;

        if (isalpha(current_char)) {
            std::string id = identifier();
            skip_whitespace();
            if (current_char == '=') {  // Variable assignment
                advance();
                int value = expr();
                variables[id] = value;
            } else if (current_char == '(') {  // Function call
                call_function(id);
            } else if (current_char == '[') {  // Array assignment
                advance();
                int index = expr();
                if (current_char != ']') error("Expected ']' for array assignment");
                advance();
                skip_whitespace();
                if (current_char != '=') error("Expected '=' for array assignment");
                advance();
                int value = expr();
                if (arrays.count(id)) {
                    if (index < 0 || index >= arrays[id].size()) error("Array index out of bounds");
                    arrays[id][index] = value;
                } else {
                    error("Undefined array: " + id);
                }
            } else {
                error("Invalid statement");
            }
        } else if (current_char == 'f') {  // Function declaration
            advance();
            std::string keyword = identifier();
            if (keyword == "function") {
                std::string func_name = identifier();
                skip_whitespace();
                if (current_char != '(') error("Expected '(' for function declaration");
                advance();

                std::vector<std::string> parameters;
                if (current_char != ')') {
                    while (true) {
                        parameters.push_back(identifier());
                        if (current_char == ')') break;
                        if (current_char != ',') error("Expected ',' between parameters");
                        advance();
                    }
                }
                advance();

                skip_whitespace();
                std::string body = "";
                if (current_char != '{') error("Expected '{' for function body");
                advance();
                while (current_char != '\0' && current_char != '}') {
                    body += current_char;
                    advance();
                }
                if (current_char != '}') error("Expected '}' to end function body");
                advance();

                functions[func_name] = {parameters, body};
            } else {
                error("Unknown keyword: " + keyword);
            }
        } else if (current_char == 'a') {  // Array declaration
            advance();
            std::string keyword = identifier();
            if (keyword == "array") {
                std::string array_name = identifier();
                skip_whitespace();
                if (current_char != '[') error("Expected '[' for array declaration");
                advance();
                int size = expr();
                if (current_char != ']') error("Expected ']' after array size");
                advance();
                arrays[array_name] = std::vector<int>(size);
            } else {
                error("Unknown keyword: " + keyword);
            }
        } else {
            error("Unknown statement");
        }
    }

    void program() {
        while (current_char != '\0') {
            statement();
            skip_whitespace();
            if (current_char == ';') {
                advance();
            } else if (current_char != '\0') {
                error("Expected ';' after statement");
            }
        }
    }

public:
    void interpret(const std::string& text) {
        input = text;
        pos = 0;
        current_char = input[pos];
        try {
            program();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
};

int main() {
    Interpreter interpreter;
    std::string code;

    std::cout << "Enter your program (end with an empty line):" << std::endl;
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) break;
        code += line + " ";
    }

    std::cout << "Executing program...\n";
    interpreter.interpret(code);

    return 0;
}
