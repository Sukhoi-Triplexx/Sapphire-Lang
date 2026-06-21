#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <map>

using std::unique_ptr;
using std::vector;
using std::string;
using std::move;
std::map<std::string, int> runtime_memory;

enum LangType
{
    TOKEN_WHILE,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_WRITE,
    TOKEN_EOF
};

struct Token
{
    LangType type;
    string value;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual int execute() = 0;
};

class NumberNode : public ASTNode {
public: 
    string value;
    NumberNode(string val) : value(val){}

    int execute() override {
        return std::stoi(value);
    }
};

class VariableNode : public ASTNode {
public:
    string name;
    VariableNode(string nm): name(nm){}

    int execute() override {
        return runtime_memory[name];
    }
};
class AssignNode : public ASTNode {
public:
    unique_ptr<ASTNode> var;
    unique_ptr<ASTNode> expr;

    AssignNode(unique_ptr<ASTNode> v, unique_ptr<ASTNode> e)
        : var(move(v)), expr(move(e)) {}

    int execute() override{
        VariableNode* v = dynamic_cast<VariableNode*>(var.get());
    if (v) {
        int val = expr->execute();
        runtime_memory[v->name] = val;
        }
    return 0;
    }
};
class PrintNode : public ASTNode {
public:
    string varName;
    PrintNode(string name) : varName(name){}
    int execute() override {
        std::cout << "[Sapphire IO]:" << runtime_memory[varName] << std::endl;
        return 0;
    }
};
class BlockNode : public ASTNode {
public:
    vector<unique_ptr<ASTNode>> statements;
    int execute() override {
        for (const auto& stmt : statements) {
            if (stmt) stmt->execute();
        }
        return 0;
    }
};
class WhileNode : public ASTNode {
public:
    unique_ptr<ASTNode> condition;
    unique_ptr<ASTNode> body;

    WhileNode(unique_ptr<ASTNode> cond, unique_ptr<ASTNode> b)
        : condition(move(cond)), body(move(b)) {}

    int execute() override{
        while (condition->execute() != 0){
            body->execute();
            
        }
        return 0;
    }
};

class Parser {
    vector<Token> tokens;
    size_t pos = 0;

    Token currentToken() {
        if (pos < tokens.size()) return tokens[pos];
        return { TOKEN_EOF, "" };
    }

    void consume() {
        pos++;
    }
public:
    Parser(vector<Token> t) : tokens(move(t)) {}
    unique_ptr<ASTNode> parseAssign() {
        consume();

        if (currentToken().type == TOKEN_ID) {
            string varName = currentToken().value;

            auto varNode = std::make_unique<VariableNode>(varName);
            consume();

            if (currentToken().type == TOKEN_ASSIGN) {
                consume();

                if (currentToken().type == TOKEN_NUMBER) {
                    string numVal = currentToken().value;

                    auto numNode = std::make_unique<NumberNode>(numVal);
                    consume();

                    if (currentToken().type == TOKEN_SEMICOLON) {
                        consume();
                        return std::make_unique <AssignNode>(move(varNode), move(numNode));
                    }
                }
            }
        }
        std::cout << "[Sapphire err0r]: let expressiv error " << std::endl;
        return nullptr;
    }
    unique_ptr<ASTNode> parsePrint() {
        consume();
        if (currentToken().type == TOKEN_LPAREN) {
            consume();

            if (currentToken().type == TOKEN_ID) {
                string varName = currentToken().value;
                consume();

                if (currentToken().type == TOKEN_RPAREN) {
                    consume();

                    if (currentToken().type == TOKEN_SEMICOLON) {
                        consume();

                        return std::make_unique<PrintNode>(varName);
                    }
                }
            }
            std::cout << "[Sapphire err0r]: Uncorrect syntax write!" << std::endl;
            return nullptr;
        }
    }
    unique_ptr<ASTNode> parseWhile() {
        consume();
        if (currentToken().type == TOKEN_LPAREN) {
            consume();

            unique_ptr<ASTNode> cond = nullptr;
            if (currentToken().type == TOKEN_NUMBER) {
                cond = std::make_unique<NumberNode>(currentToken().value);
                consume();
            }
            else if (currentToken().type == TOKEN_ID) {
                cond = std::make_unique<VariableNode>(currentToken().value);
                consume();
            }

            if (currentToken().type == TOKEN_RPAREN) {
                consume();
                if (currentToken().type == TOKEN_LBRACE) {
                    consume();

                    auto bodyBlock = std::make_unique<BlockNode>();

                    while (currentToken().type != TOKEN_RBRACE && currentToken().type != TOKEN_EOF) {
                        unique_ptr<ASTNode> stmt = nullptr;
                        if (currentToken().type == TOKEN_LET) stmt = parseAssign();
                        else if (currentToken().type == TOKEN_WRITE) stmt = parsePrint();

                        if (stmt) {
                            bodyBlock->statements.push_back(move(stmt));
                        } else {
                            consume();
                        }
                        
                    }
                    if (currentToken().type == TOKEN_RBRACE) {
                        consume();
                        return std::make_unique<WhileNode>(move(cond), move(bodyBlock));
                    }
                }
            }
        }
        std::cout << "[Sapphire error]: Invalid while syntax" << std::endl;
        return nullptr;
    }
    unique_ptr<ASTNode> parseCode() {
        auto block = std::make_unique<BlockNode>();
        {
            while (currentToken().type != TOKEN_EOF) {
                unique_ptr<ASTNode> stmt = nullptr;

                if (currentToken().type == TOKEN_LET)
                {
                    stmt = parseAssign();
                }
                else if (currentToken().type == TOKEN_WRITE)
                {
                    stmt = parsePrint();
                }
                else if (currentToken().type == TOKEN_WHILE) stmt = parseWhile();
                else
                {
                    std::cout << "[Sapphire err0r]: Unkown token!" << std::endl;
                    consume();
                }

                if (stmt != nullptr)
                {
                    block->statements.push_back(move(stmt));
                }
            }
            return block;
        }
    }
};
vector<Token> tokenize(const string& code) {
    vector<Token> tokens;
    size_t i = 0;

    while (i < code.length()) {

        if (isspace(code[i])) {
            i++;
            continue;
        }

        if (code[i] == '=')
        {
            tokens.push_back({ TOKEN_ASSIGN, "=" });
            i++; continue;
        }

        if (code[i] == ';') {
            tokens.push_back({ TOKEN_SEMICOLON, ";" });
            i++; continue;
        }

        if (code[i] == '{') {
            tokens.push_back({ TOKEN_LBRACE, "{" });
            i++; continue;
        }

        if (code[i] == '}') {
            tokens.push_back({ TOKEN_RBRACE, "}" });
            i++; continue;
        }

        if (code[i] == '(') {
            tokens.push_back({ TOKEN_LPAREN, "(" });
            i++; continue;
        }

        if (code[i] == ')') {
            tokens.push_back({ TOKEN_RPAREN, ")" });
            i++; continue;
        }


        if (isdigit(code[i])) {
            string num = "";
            while (i < code.length() && isdigit(code[i])) {
                num += code[i];
                i++;
            }
            tokens.push_back({ TOKEN_NUMBER, num });
            continue;

        }
        if (isalpha(code[i])) {
            string word = "";
            while (i < code.length() && isalpha(code[i])) {
                word += code[i];
                i++;
            }

            if (word == "while") tokens.push_back({ TOKEN_WHILE, word });
            else if (word == "let") tokens.push_back({ TOKEN_LET, word });
            else if (word == "then") tokens.push_back({ TOKEN_THEN, word });
            else if (word == "write") tokens.push_back({ TOKEN_WRITE, word });
            else if (word == "if") tokens.push_back({ TOKEN_IF, word });
            else tokens.push_back({ TOKEN_ID, word });
            continue;
        }
    }
    tokens.push_back({ TOKEN_EOF, "" });
    return tokens;
}

int main()
{
    string code = "let x = 40;while(1){write(x);}";
    vector<Token>tokens = tokenize(code);
    Parser parser(move(tokens));
    unique_ptr<ASTNode> root = parser.parseCode();
    std::cout << "Sapphire out\n";
    if (root) {
        int result = root->execute();
        std::cout << "[Sapphire return]: " << result << std::endl;
    }

    return 0;
}
