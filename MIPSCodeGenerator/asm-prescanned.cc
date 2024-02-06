#include <vector>
#include <iostream>
#include <string>
#include <map>

using namespace std;

enum class TokenKind {
    ID, DOTID, LABELDEF, DECINT, HEXINT, REGISTER, COMMA, LPAREN, RPAREN, NEWLINE
};

struct Token {
    TokenKind kind;
    string lexeme;
    explicit Token (const TokenKind kind, const string lexeme): kind{kind}, lexeme{lexeme} {}
};

// struct Label {
//     string name;
//     int address;
//     explicit Label (const string name, const int address): name{name}, address{address} {}
//     bool operator== (const string other) const {
//         if (name == other)  return true;
//         return false;
//     }
// };

TokenKind tKindConverter (const string kind) {
    if (kind == "ID")  return TokenKind::ID;
    else if (kind == "DOTID")  return TokenKind::DOTID;
    else if (kind == "LABELDEF")  return TokenKind::LABELDEF;
    else if (kind == "DECINT")  return TokenKind::DECINT;
    else if (kind == "HEXINT")  return TokenKind::HEXINT;
    else if (kind == "REGISTER")  return TokenKind::REGISTER;
    else if (kind == "COMMA")  return TokenKind::COMMA;
    else if (kind == "LPAREN")  return TokenKind::LPAREN;
    else if (kind == "RPAREN")  return TokenKind::RPAREN;
    else if (kind == "NEWLINE") return TokenKind::NEWLINE;
    else throw runtime_error ("token kind not found");
}

// converting string num (DECINT) to int value
int decConverter (const string num) {
    int acc = 0;
    int len = num.length();
    if (num[0] != '-') { // unsigned DECINT
        for (int i = 0; i < len; ++i) {
            acc *= 10;
            acc += num[i] - '0';
        }
    }
    else { // negative signed DECINT
        for (int i = 1; i < len; ++i) {
            acc *= 10;
            acc -= num[i] - '0';
        }
    }
    return acc;
}

int hexValue (const char num) {
    switch (num) {
        case 'A':
        case 'a': return 10;
        case 'B':
        case 'b': return 11;
        case 'C':
        case 'c': return 12;
        case 'D':
        case 'd': return 13;
        case 'E':
        case 'e': return 14;
        case 'F':
        case 'f': return 15;
        default: return num - '0';
    }
}

// converting string num (HEXINT) to int value
int hexConverter (const string num) {
    int acc = 0;
    int len = num.length();
    for (int i = 2; i < len; ++i) {
        acc *= 16;
        acc += hexValue(num[i]);
    }
    return acc;
}

int regConverter (const string num) {
    int acc = 0;
    int len = num.length();
    for (int i = 1; i < len; ++i) {
        acc *= 10;
        acc += num[i] - '0';
    }
    return acc;
}

// // Return the address of the label. If not found, return -1;
// int getLabelAddress (const vector<Label> &labels, const string labelName) {
//     int len = labels.size();
//     for (int i = 0; i < len; ++i) {
//         if (labels[i] == labelName)  return labels[i].address;
//     }
//     return -1;
// }

// Check syntax errors. If an error is found, throw an runtime error
void checkSyntax (const vector<Token> &line) {
    // Find the index for first non-label token
    int index = 0;
    int lineLen = line.size();

    // Check the syntax error
    Token firstInstr = line[0];
    switch (firstInstr.kind) {

        case TokenKind::DOTID: {
            if (firstInstr.lexeme != ".word") 
                throw runtime_error ("DOTID token unrecognized: " + firstInstr.lexeme);
            if (lineLen <= 1)  
                throw runtime_error ("expecting more tokens for .word, found none");
            else if (lineLen > 2)  
                throw runtime_error ("too many tokens found while processing .word");
            else {
                // Read next token
                index += 1;
                Token tk = line[index];
                if (tk.kind != TokenKind::DECINT && 
                    tk.kind != TokenKind::HEXINT &&
                    tk.kind != TokenKind::ID)
                    throw runtime_error ("unexpected token " + tk.lexeme + " while processing .word");
            }
            break;
        }

        case TokenKind::ID: {
            if (firstInstr.lexeme == "add" ||
                firstInstr.lexeme == "sub" ||
                firstInstr.lexeme == "slt" ||
                firstInstr.lexeme == "sltu") {

                // Check this line have the right number of tokens
                if (lineLen != 6)
                    throw runtime_error ("wrong number of tokens for " + firstInstr.lexeme);
                
                // Check the kind of tokens are all expected
                if (line[1].kind != TokenKind::REGISTER ||
                    line[2].kind != TokenKind::COMMA ||
                    line[3].kind != TokenKind::REGISTER ||
                    line[4].kind != TokenKind::COMMA ||
                    line[5].kind != TokenKind::REGISTER)
                    throw runtime_error ("wrong kinds of tokens for " + firstInstr.lexeme);
            }

            else if (firstInstr.lexeme == "beq" ||
                     firstInstr.lexeme == "bne") {
                
                // Check this line have the right number of tokens
                if (lineLen != 6)
                    throw runtime_error ("wrong number of tokens for " + firstInstr.lexeme);
                
                // Check the kind of tokens are all expected
                if (line[1].kind != TokenKind::REGISTER ||
                    line[2].kind != TokenKind::COMMA ||
                    line[3].kind != TokenKind::REGISTER ||
                    line[4].kind != TokenKind::COMMA ||
                    (line[5].kind != TokenKind::DECINT &&
                     line[5].kind != TokenKind::HEXINT &&
                     line[5].kind != TokenKind::ID))
                    throw runtime_error ("wrong kinds of tokens for " + firstInstr.lexeme);
            }

            else if (firstInstr.lexeme == "mult" ||
                     firstInstr.lexeme == "multu" ||
                     firstInstr.lexeme == "div" ||
                     firstInstr.lexeme == "divu") {
                
                // Check this line have the right number of tokens
                if (lineLen != 4)
                    throw runtime_error ("wrong number of tokens for " + firstInstr.lexeme);
                
                // Check the kind of tokens are all expected
                if (line[1].kind != TokenKind::REGISTER ||
                    line[2].kind != TokenKind::COMMA ||
                    line[3].kind != TokenKind::REGISTER)
                    throw runtime_error ("wrong kinds of tokens for " + firstInstr.lexeme);
                
            }

            else if (firstInstr.lexeme == "mfhi" ||
                     firstInstr.lexeme == "mflo" ||
                     firstInstr.lexeme == "lis" ||
                     firstInstr.lexeme == "jr" ||
                     firstInstr.lexeme == "jalr") {
                
                // Check this line have the right number of tokens
                if (lineLen != 2)
                    throw runtime_error ("wrong number of tokens for " + firstInstr.lexeme);
                
                // Check the kind of tokens are all expected
                if (line[1].kind != TokenKind::REGISTER)
                    throw runtime_error ("wrong kinds of tokens for " + firstInstr.lexeme);
            }

            else if (firstInstr.lexeme == "lw" ||
                     firstInstr.lexeme == "sw") {
                
                // Check this line have the right number of tokens
                if (lineLen != 7)
                    throw runtime_error ("wrong number of tokens for " + firstInstr.lexeme);
                
                // Check the kind of tokens are all expected
                if (line[1].kind != TokenKind::REGISTER ||
                    line[2].kind != TokenKind::COMMA ||
                    (line[3].kind != TokenKind::DECINT &&
                    line[3].kind != TokenKind::HEXINT) ||
                    line[4].kind != TokenKind::LPAREN ||
                    line[5].kind != TokenKind::REGISTER ||
                    line[6].kind != TokenKind::RPAREN)
                    throw runtime_error ("wrong kinds of tokens for " + firstInstr.lexeme);
            }

            else throw runtime_error ("Invalid opcode " + firstInstr.lexeme);
            break;
        }
        default: {
            throw runtime_error ("Invalid instruction component " + firstInstr.lexeme);
        }
    }
}

// Read Tokens from cin
void readTokens (vector<vector<Token>> &tokens, map<string, int> &labels) {
    string w;
    int lineIndex = 0;
    while (cin >> w) {

        // Read labels
        TokenKind tk = tKindConverter(w);
        while (tk == TokenKind::LABELDEF) {

            cin >> w;
            w.pop_back();

            // Check duplicity
            auto it = labels.find(w);
            if (it != labels.end())
                throw runtime_error ("Duplicate label " + w);
            
            // store label
            labels.insert({w, lineIndex * 4});
            cin >> w;
            tk = tKindConverter(w);
        }

        // First Non-Label Token in this line
        if (tk == TokenKind::NEWLINE)  continue;

        // Not newline, create a vector representing the line
        vector<Token> line;
        cin >> w;
        line.push_back(Token(tk, w));

        // Read other tokens for this line
        while (cin >> w) {
            TokenKind kind = tKindConverter(w);
            if (kind == TokenKind::NEWLINE)  break;
            cin >> w;
            line.push_back(Token(kind, w));
        }

        // Check the syntax for this line
        checkSyntax(line);
        tokens.push_back(line);
        lineIndex += 1;
    }
}

// Output the 32-bit instruction code
void outputInstr (int instr) {
    for (int i = 24; i >= 0; i -= 8) {
        char byte = (instr >> i) & 0xFF;
        cout << byte;
    }
}

int encode (const vector<Token> &line, const map<string, int> &labels, const int lineIndex) {
    int instr = 0;
    int index = 0;

    Token firstInstr = line[index];
    switch (firstInstr.kind) {
        case TokenKind::DOTID: {
            // Read next token
            index += 1;
            Token tk = line[index];
            switch (tk.kind) {
                case TokenKind::DECINT: {
                    instr = decConverter(tk.lexeme);
                    break;
                }
                case TokenKind::HEXINT: {
                    instr = hexConverter(tk.lexeme);
                    break;
                }
                case TokenKind::ID: {
                    auto it = labels.find(tk.lexeme);
                    // Check if the label is defined
                    if (it == labels.end())
                        throw runtime_error ("Use of undefined label " + tk.lexeme + " in .word");
                    
                    instr = it->second;
                    break;
                }
                default: {
                    throw runtime_error ("unexpected token " + tk.lexeme + " while processing .word");
                }
            }
            break;
        }

        case TokenKind::ID: {

            if (firstInstr.lexeme == "add" ||
                firstInstr.lexeme == "sub" ||
                firstInstr.lexeme == "slt" ||
                firstInstr.lexeme == "sltu") {
                // Add register code
                instr = (0 << 26) | 
                        (regConverter(line[3].lexeme) << 21) |
                        (regConverter(line[5].lexeme) << 16) |
                        (regConverter(line[1].lexeme) << 11) | 0;
                // Add function code
                if (firstInstr.lexeme == "add")  instr |= 32;
                else if (firstInstr.lexeme == "sub")  instr |= 34;
                else if (firstInstr.lexeme == "slt")  instr |= 42;
                else if (firstInstr.lexeme == "sltu")  instr |= 43;
            }
            
            else if (firstInstr.lexeme == "beq" ||
                     firstInstr.lexeme == "bne"){
                // Adding register values
                instr = (regConverter(line[1].lexeme) << 21) |
                        (regConverter(line[3].lexeme) << 16) | 0;
                
                // Adding opcode
                if (firstInstr.lexeme == "beq")  instr |= (4 << 26);
                else  instr |= (5 << 26);

                // encoding i
                int16_t encodedI = 0;
                if (line[5].kind == TokenKind::DECINT) {

                    int value = decConverter(line[5].lexeme);
                    // this DECINT is a positive number
                    if (line[5].lexeme[0] != '-') {
                        if (value > 32767)
                            throw runtime_error ("Out-of-range immediate value " +
                                                 line[5].lexeme + " in instruction "
                                                 + line[0].lexeme);
                        
                        encodedI = value;
                    }
                    // this DECINT is a negative number
                    else {
                        if (value < -32768)
                            throw runtime_error ("Out-of-range immediate value " +
                                                 line[5].lexeme + " in instruction "
                                                 + line[0].lexeme);
                        
                        encodedI = value;
                    }
                }
                else if (line[5].kind == TokenKind::HEXINT) {
                    unsigned int value = hexConverter(line[5].lexeme);
                    if (value > 0xFFFF)
                        throw runtime_error ("Out-of-range immediate value " +
                                             line[5].lexeme + " in instruction "
                                             + line[0].lexeme);
                        
                    encodedI = value;
                }
                else { // line[5] is a label
                    auto it = labels.find(line[5].lexeme);
                    // Address do not exist, throw error
                    if (it == labels.end())
                        throw runtime_error ("undefined label " + line[5].lexeme);
                    
                    // Calculate i
                    int intI = it->second/4 - (lineIndex + 1);

                    // Check if i is in range
                    if (intI < -32768 || intI > 32767)
                        throw runtime_error ("Out-of-range immediate value " +
                                             line[5].lexeme + " in instruction "
                                             + line[0].lexeme);
                    
                    encodedI = intI;
                }
                // encode i into instr
                instr |= (encodedI & 0xFFFF);
            }

            else if (firstInstr.lexeme == "mult" ||
                     firstInstr.lexeme == "multu" ||
                     firstInstr.lexeme == "div" ||
                     firstInstr.lexeme == "divu") {
                
                // Adding register code
                instr = (0 << 26) |
                        (regConverter(line[1].lexeme) << 21) |
                        (regConverter(line[3].lexeme) << 16) | 0;
                
                // Adding function code
                if (firstInstr.lexeme == "mult")  instr |= 24;
                else if (firstInstr.lexeme == "multu")  instr |= 25;
                else if (firstInstr.lexeme == "div")  instr |= 26;
                else  instr |= 27;
            }

            else if (firstInstr.lexeme == "mfhi" ||
                     firstInstr.lexeme == "mflo" ||
                     firstInstr.lexeme == "lis") {
                
                // Adding register code
                instr = (0 << 26) |
                        (regConverter(line[1].lexeme) << 11) | 0;
                
                // Adding function code
                if (firstInstr.lexeme == "mfhi")  instr |= 16;
                else if (firstInstr.lexeme == "mflo")  instr |= 18;
                else  instr |= 20;
            }

            else if (firstInstr.lexeme == "jr" ||
                     firstInstr.lexeme == "jalr") {
                
                // Adding register code
                instr = (0 << 26) |
                        (regConverter(line[1].lexeme) << 21) | 0;
                
                // Adding function code
                if (firstInstr.lexeme == "jr")  instr |= 8;
                else  instr |= 9;
            }

            else if (firstInstr.lexeme == "lw" ||
                     firstInstr.lexeme == "sw") {
            
                // Adding register values
                instr = (regConverter(line[5].lexeme) << 21) |
                        (regConverter(line[1].lexeme) << 16) | 0;
                
                // Adding opcode
                if (firstInstr.lexeme == "lw")  instr |= (35 << 26);
                else instr |= (43 << 26);

                // encoding i
                // encoding i
                int16_t encodedI = 0;
                if (line[3].kind == TokenKind::DECINT) {

                    int value = decConverter(line[3].lexeme);
                    // this DECINT is a positive number
                    if (line[3].lexeme[0] != '-') {
                        if (value > 32767)
                            throw runtime_error ("Out-of-range immediate value " +
                                                 line[3].lexeme + " in instruction "
                                                 + line[0].lexeme);
                        
                        encodedI = value;
                    }
                    // this DECINT is a negative number
                    else {
                        if (value < -32768)
                            throw runtime_error ("Out-of-range immediate value " +
                                                 line[3].lexeme + " in instruction "
                                                 + line[0].lexeme);
                        
                        encodedI = value;
                    }
                }
                else {
                    unsigned int value = hexConverter(line[3].lexeme);
                    if (value > 0xFFFF)
                        throw runtime_error ("Out-of-range immediate value " +
                                             line[3].lexeme + " in instruction "
                                             + line[0].lexeme);
                        
                    encodedI = value;
                }
                // encode i into instr
                instr |= (encodedI & 0xFFFF);
            }
            else
                throw runtime_error ("Invalid instruction component " + firstInstr.lexeme);
            break;
        }
        default: {
            throw runtime_error ("Invalid instruction component " + firstInstr.lexeme);
        }
    }
    return instr;
}

int main () {
    try {
        vector<vector<Token>> lines;
        map<string, int> labels;

        readTokens(lines, labels);

        int numLines = lines.size();
        for (int i = 0; i < numLines; ++i) {
            // encode and output each line
            int instr = encode(lines[i], labels, i);
            outputInstr(instr);
        }
    }
    catch (runtime_error &e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
}
