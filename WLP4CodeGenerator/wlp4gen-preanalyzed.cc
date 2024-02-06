#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
using namespace std;

const string EMPTY = ".EMPTY";
const string COLON = ":";

vector<int> labelCounts(4, 0);
string getLabel(string labelType) {
    if (labelType == "while") {
        labelCounts[0] += 1;
        return "while" + to_string(labelCounts[0]);
    }
    else if (labelType == "endwhile") {
        labelCounts[1] += 1;
        return "endwhile" + to_string(labelCounts[1]);
    }
    else if (labelType == "else") {
        labelCounts[2] += 1;
        return "else" + to_string(labelCounts[2]);
    }
    else if (labelType == "endif") {
        labelCounts[3] += 1;
        return "endif" + to_string(labelCounts[3]);
    }
    else throw runtime_error("invalid labelTyle in getLabel");
}

enum class TypeVariable { INT, PINT, NOTYPE };
ostream &operator<<(ostream &out, const TypeVariable &tv) {
    switch (tv) {
        case (TypeVariable::INT):
            out << " : int";
            break;
        case (TypeVariable::PINT):
            out << " : int*";
            break;
        default:
            break;
    }
    return out;
}

struct Node {
    string rule;
    string lhs;
    string kind;
    string lexeme;
    vector<Node *> children;
    TypeVariable type = TypeVariable::NOTYPE;
    Node(istream &in) {
        string line;
        getline(in, line);
        istringstream iss{line};
        if (line[0] >= 'A' && line[0] <= 'Z') { // line is a terminal node
            iss >> kind >> lexeme;
            rule = kind + ' ' + lexeme;
            string nodeType;
            if (iss >> nodeType >> nodeType) {
                if (nodeType == "int") type = TypeVariable::INT;
                else type = TypeVariable::PINT;
            }
        }
        else { // line is a nonterminal node
            iss >> lhs;
            rule = lhs;
            string rhs;
            while (iss >> rhs) {
                if (rhs == COLON) {
                    break;
                }
                rule += ' ';
                rule += rhs;
                if (rhs != EMPTY) {
                    Node *child = new Node(in);
                    children.push_back(child);
                }
            }
            if (iss >> rhs) {
                if (rhs == "int") type = TypeVariable::INT;
                else type = TypeVariable::PINT;
            }
        }
    }
    Node *getChild(string name, int index) {
        int count = index;
        for ( auto &n : children ) {
            if (n->kind != "") {
                if (n->kind == name) --count;
            }
            else {
                if (n->lhs == name) --count;
            }
            if (count <= 0) return n;
        }
        return nullptr;
    }
    void print() {
        if (kind[0] <= 'Z' && kind[0] >= 'A') {
            std::cout << kind << ' ' << lexeme << type << endl;
        } else {
            std::cout << rule << type << endl;
        }
        // cout << rule << kind << ' ' << lexeme
        //      << type << endl;
        for ( auto &n : children ) { n->print(); }
    }
    ~Node() {
        for ( auto &n : children ) { delete n; }
    }
};

// output helper functions
void Add(int d, int s, int t) { 
    std::cout << "add $" << d << ", $" << s << ", $" << t << "\n"; 
}
void Sub(int d, int s, int t) { 
    std::cout << "sub $" << d << ", $" << s << ", $" << t << "\n"; 
}
void Mult(int s, int t) {
    std::cout << "mult $" << s << ", $" << t << "\n";
}
void Multu(int s, int t) {
    std::cout << "multu $" << s << ", $" << t << "\n";
}
void Div(int s, int t) {
    std::cout << "div $" << s << ", $" << t << "\n";
}
void Divu(int s, int t) {
    std::cout << "divu $" << s << ", $" << t << "\n";
}
void Mfhi(int d) {
    std::cout << "mfhi $" << d << "\n";
}
void Mflo(int d) {
    std::cout << "mflo $" << d << "\n";
}
void Lis(int d) {
    std::cout << "lis $" << d << "\n";
}
void Slt(int d, int s, int t) { 
    std::cout << "slt $" << d << ", $" << s << ", $" << t << "\n"; 
}
void Sltu(int d, int s, int t) { 
    std::cout << "sltu $" << d << ", $" << s << ", $" << t << "\n"; 
}
void Jr(int s) { 
    std::cout << "jr $" << s << "\n";
}
void Jalr(int s) { 
    std::cout << "jalr $" << s << "\n";
}
void Beq(int s, int t, std::string label) { 
    std::cout << "beq $" << s << ", $" << t << ", " + label + "\n"; 
}
void Beq(int s, int t, int i) { 
    std::cout << "beq $" << s << ", $" << t << ", " << i << endl; 
}
void Bne(int s, int t, std::string label) { 
    std::cout << "bne $" << s << ", $" << t << ", " + label + "\n"; 
}
void Bne(int s, int t, int i) { 
    std::cout << "bne $" << s << ", $" << t << ", " << i << endl; 
}
void Lw(int s, int t, int i) {
    std::cout << "lw $" << t << ", " << i << "($" << s << ")\n";
}
void Sw(int s, int t, int i) {
    std::cout << "sw $" << t << ", " << i << "($" << s << ")\n";
}
void Word(int i) {
    std::cout << ".word " << i << "\n";
}
void Word(std::string label) {
    std::cout << ".word " + label + "\n";
}
void Label(std::string name) {
    std::cout << name + ":\n";
}
void push(int s) {
    std::cout << "sw $" << s << ", -4($30)\n";
    std::cout << "sub $30, $30, $4\n";
}
void pop(int d) {
    std::cout << "add $30, $30, $4\n";
    std::cout << "lw $" << d << ", -4($30)\n";
}
void pop() {
    std::cout << "add $30, $30, $4\n";
}

// code(lhs)
void codeExpr(Node *n, map<string, int> &offsetTable);
void codeTerm(Node *n, map<string, int> &offsetTable);
void codeFactor(Node *n, map<string, int> &offsetTable);
void codeLvalue(Node *n, map<string, int> &offsetTable);
void codeStatements(Node *n, map<string, int> &offsetTable);
void codeStatement(Node *n, map<string, int> &offsetTable);
void codeTest(Node *n, map<string, int> &offsetTable);

void codeExpr(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "expr term") {
        codeTerm(n->getChild("term", 1), offsetTable);
    }
    else if (n->rule == "expr expr PLUS term") {
        if (n->getChild("expr", 1)->type == TypeVariable::PINT) { // int* + int
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Multu(3, 4);
            Mflo(3);
            Add(3, 5, 3);
        }
        else if (n->getChild("term", 1)->type == TypeVariable::PINT) { // int + int*
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Multu(5, 4);
            Mflo(5);
            Add(3, 5, 3);
        }
        else { // int + int
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Add(3, 5, 3);
        }
    }
    else if (n->rule == "expr expr MINUS term") {
        if (n->getChild("term", 1)->type == TypeVariable::PINT) { // int* - int*
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Sub(3, 5, 3);
            Divu(3, 4);
            Mflo(3);
        }
        else if (n->getChild("expr", 1)->type == TypeVariable::PINT) { // int* - int
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Multu(3, 4);
            Mflo(3);
            Sub(3, 5, 3);
        }
        else { // int - int
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeTerm(n->getChild("term", 1), offsetTable);
            pop(5);
            Sub(3, 5, 3);
        }
        
    }
    else throw runtime_error("expr");
}
void codeTerm(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "term factor") {
        codeFactor(n->getChild("factor", 1), offsetTable);
    }
    else if (n->rule == "term term STAR factor") {
        codeTerm(n->getChild("term", 1), offsetTable);
        push(3);
        codeFactor(n->getChild("factor", 1), offsetTable);
        pop(5);
        Mult(5, 3);
        Mflo(3);
    }
    else if (n->rule == "term term SLASH factor") {
        codeTerm(n->getChild("term", 1), offsetTable);
        push(3);
        codeFactor(n->getChild("factor", 1), offsetTable);
        pop(5);
        Div(5, 3);
        Mflo(3);
    }
    else if (n->rule == "term term PCT factor") {
        codeTerm(n->getChild("term", 1), offsetTable);
        push(3);
        codeFactor(n->getChild("factor", 1), offsetTable);
        pop(5);
        Div(5, 3);
        Mfhi(3);
    }
    else throw runtime_error("term");
}
void codeFactor(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "factor ID") {
        auto it = offsetTable.find(n->children[0]->lexeme);
        Lw(29, 3, it->second);
    }
    else if (n->rule == "factor NUM") {
        string numStr = n->getChild("NUM", 1)->lexeme;
        istringstream iss{numStr};
        int num;
        if (iss >> num) {
            Lis(3);
            Word(num);
        }
        else {
            throw runtime_error("factor NUM");
        }
    }
    else if (n->rule == "factor NULL") {
        Lis(3);
        Word(1);
    }
    else if (n->rule == "factor LPAREN expr RPAREN") {
        codeExpr(n->getChild("expr", 1), offsetTable);
    }
    else if (n->rule == "factor AMP lvalue") {
        codeLvalue(n->getChild("lvalue", 1), offsetTable);
    }
    else if (n->rule == "factor STAR factor") {
        codeFactor(n->getChild("factor", 1), offsetTable);
        Lw(3, 3, 0);
    }
    else if (n->rule == "factor NEW INT LBRACK expr RBRACK") {
        // calculate the size to be allocated
        codeExpr(n->getChild("expr", 1), offsetTable);
        
        // call the new procedure
        push(31);
        push(1);
        Add(1, 3, 0);
        Lis(3);
        Word("new");
        Jalr(3);
        pop(1);
        pop(31);

        // check if it returns nullptr
        Bne(3, 0, 2);
        Lis(3);
        Word(1);
    }
    else if (n->rule == "factor ID LPAREN RPAREN") {
        // get the ID of called procedure
        string procedureName = n->children[0]->lexeme;

        // store $29 and $31, modify $29
        push(31);
        push(29);
        Sub(29, 30, 4);

        // call the procedure
        Lis(3);
        Word('P' + procedureName);
        Jalr(3);

        // restore $29 and $31
        pop(29);
        pop(31);
    }
    else if (n->rule == "factor ID LPAREN arglist RPAREN") {
        // get the ID of called procedure
        string procedureName = n->children[0]->lexeme;

        // store $31
        push(31);
        push(29);
        // push the params to the stack
        int nParam = 0;
        Node *arglist = n->getChild("arglist", 1);
        while (true) {
            codeExpr(arglist->getChild("expr", 1), offsetTable);
            push(3);
            ++nParam;
            arglist = arglist->getChild("arglist", 1);
            if (!arglist) break;
        }

        // call the procedure
        
        Sub(29, 30, 4);
        Lis(3);
        Word('P' + procedureName);
        Jalr(3);
        

        // pop the params from the stack
        for (int i = 0; i < nParam; ++i) {
            pop();
        }

        // restore $31
        pop(29);
        pop(31);
        
    }
    else throw runtime_error("factor");
}

void codeLvalue(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "lvalue ID") {
        auto it = offsetTable.find(n->children[0]->lexeme);
        Lis(3);
        Word(it->second);
        Add(3, 29, 3);
    }
    else if (n->rule == "lvalue LPAREN lvalue RPAREN") {
        codeLvalue(n->getChild("lvalue", 1), offsetTable);
    }
    else if (n->rule == "lvalue STAR factor") {
        codeFactor(n->getChild("factor", 1), offsetTable);
    }
    else throw runtime_error("lvalue");
}

void codeTest(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "test expr EQ expr") {
        codeExpr(n->getChild("expr", 1), offsetTable);
        push(3);
        codeExpr(n->getChild("expr", 2), offsetTable);
        pop(5);
        Beq(3, 5, 2);
        Add(3, 0, 0);
        Beq(0, 0, 2);
        Lis(3);
        Word(1);
    }
    else if (n->rule == "test expr NE expr") {
        codeExpr(n->getChild("expr", 1), offsetTable);
        push(3);
        codeExpr(n->getChild("expr", 2), offsetTable);
        pop(5);
        Bne(3, 5, 2);
        Add(3, 0, 0);
        Beq(0, 0, 2);
        Lis(3);
        Word(1);
    }
    else if (n->rule == "test expr LT expr") {
        if (n->getChild("expr", 1)->type == TypeVariable::INT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            Slt(3, 5, 3);
        }
        else if (n->getChild("expr", 1)->type == TypeVariable::PINT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            Sltu(3, 5, 3);
        }
    }
    else if (n->rule == "test expr LE expr") {
        if (n->getChild("expr", 1)->type == TypeVariable::INT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            push(3);
            Slt(3, 5, 3);
            Bne(3, 0, 7); // skip the following lines if a < b
            pop(3); // 2 lines
            Beq(3, 5, 2);
            Add(3, 0, 0);
            Beq(0, 0, 2);
            Lis(3);
            Word(1);
        }
        else if (n->getChild("expr", 1)->type == TypeVariable::PINT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            push(3);
            Sltu(3, 5, 3);
            Bne(3, 0, 7); // skip the following lines if a < b
            pop(3); // 2 lines
            Beq(3, 5, 2);
            Add(3, 0, 0);
            Beq(0, 0, 2);
            Lis(3);
            Word(1);
        }
    }
    else if (n->rule == "test expr GE expr") {
        if (n->getChild("expr", 1)->type == TypeVariable::INT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            push(3);
            Slt(3, 3, 5);
            Bne(3, 0, 7); // skip the following lines if a < b
            pop(3); // 2 lines
            Beq(3, 5, 2);
            Add(3, 0, 0);
            Beq(0, 0, 2);
            Lis(3);
            Word(1);
        }
        else if (n->getChild("expr", 1)->type == TypeVariable::PINT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            push(3);
            Sltu(3, 3, 5);
            Bne(3, 0, 7); // skip the following lines if a < b
            pop(3); // 2 lines
            Beq(3, 5, 2);
            Add(3, 0, 0);
            Beq(0, 0, 2);
            Lis(3);
            Word(1);
        }
    }
    else if (n->rule == "test expr GT expr") {
        if (n->getChild("expr", 1)->type == TypeVariable::INT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            Slt(3, 3, 5);
        }
        else if (n->getChild("expr", 1)->type == TypeVariable::PINT) {
            codeExpr(n->getChild("expr", 1), offsetTable);
            push(3);
            codeExpr(n->getChild("expr", 2), offsetTable);
            pop(5);
            Sltu(3, 3, 5);
        }
    }
    else throw runtime_error("test");
}

void codeStatements(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "statements .EMPTY") {}
    else if (n->rule == "statements statements statement") {
        codeStatements(n->getChild("statements", 1), offsetTable);
        codeStatement(n->getChild("statement", 1), offsetTable);
    }
    else throw runtime_error("statements");
}

void codeStatement(Node *n, map<string, int> &offsetTable) {
    if (n->rule == "statement lvalue BECOMES expr SEMI") {
        // 3 contains the address of the lvalue
        codeLvalue(n->getChild("lvalue", 1), offsetTable);
        push(3);
        codeExpr(n->getChild("expr", 1), offsetTable);
        pop(5);
        Sw(5, 3, 0);
    }
    else if (n->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        push(31);
        push(1);
        codeExpr(n->getChild("expr", 1), offsetTable);
        Add(1, 3, 0);
        Lis(3);
        Word("print");
        Jalr(3);
        pop(1);
        pop(31);
        
    }
    else if (n->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        string elseStr = getLabel("else");
        string endifStr = getLabel("endif");

        codeTest(n->getChild("test", 1), offsetTable);

        Beq(3, 0, elseStr);

        codeStatements(n->getChild("statements", 1), offsetTable);
        Beq(0, 0, endifStr);

        Label(elseStr);
        codeStatements(n->getChild("statements", 2), offsetTable);

        Label(endifStr);
    }
    else if (n->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        string whileStr = getLabel("while");
        string endwhileStr = getLabel("endwhile");

        Label(whileStr);
        codeTest(n->getChild("test", 1), offsetTable);
        Beq(3, 0, endwhileStr);

        codeStatements(n->getChild("statements", 1), offsetTable);

        Beq(0, 0, whileStr);
        Label(endwhileStr);
    }
    else if (n->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        // calculate the address to free
        codeExpr(n->getChild("expr", 1), offsetTable);

        // check if the address is NULL
        Lis(5);
        Word(1);
        Beq(3, 5, 12);

        // call delete procedure, altogether 12 lines
        push(31);
        push(1);
        Add(1, 3, 0);
        Lis(3);
        Word("delete");
        Jalr(3);
        pop(1);
        pop(31);
    }
    else throw runtime_error("statement");
}


// code(procedure)
void codeProcedure(Node *n) {
    if (n->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        
        Label('P' + n->getChild("ID", 1)->lexeme);

        map<string, int> offsetTable;
        int nLocalVariables = 0;

        // read params
        if (n->getChild("params", 1)->rule != "params .EMPTY") {
            Node *paramlist = n->getChild("params", 1)->getChild("paramlist", 1);

            // get the amount of params
            int numParams = 0;
            Node *countParamlist = paramlist;
            while (countParamlist) {
                ++numParams;
                countParamlist = countParamlist->getChild("paramlist", 1);
            }

            // modify the offset table
            int curOffset = numParams * 4;
            while (paramlist) {
                Node *dcl = paramlist->getChild("dcl", 1);
                string vName = dcl->getChild("ID", 1)->lexeme;

                // store the variable to offsetTable
                offsetTable.insert(make_pair(vName, curOffset));
                paramlist = paramlist->getChild("paramlist", 1);
                curOffset -= 4;
            }
        }

        // read local variables
        Node *dcls = n->getChild("dcls", 1);
        int curOffset = 0;
        while (!(dcls->children.empty())) {
        
            string vName = dcls->getChild("dcl", 1)->getChild("ID", 1)->lexeme;
            string vValue = dcls->children[3]->lexeme;

            // store the variable to offsetTable
            offsetTable.insert(make_pair(vName, curOffset));

            // store the word to stack
            Lis(3);
            if (vValue == "NULL") {
                Word(1);
            } else {
                istringstream iss{vValue};
                int value;
                iss >> value;
                Word(value);
            }
            push(3);

            // recursion
            dcls = dcls->getChild("dcls", 1);
            curOffset -= 4;
            ++nLocalVariables;
        }

        // save registers
        // push(1);
        // push(2);
        // push(4);
        // push(5);
        // push(6);
        // push(7);


        // code for statements and return
        codeStatements(n->getChild("statements", 1), offsetTable);
        codeExpr(n->getChild("expr", 1), offsetTable);
        
        // restore register values, end this procedure
        // pop(7);
        // pop(6);
        // pop(5);
        // pop(4);
        // pop(2);
        // pop(1);

        // pop local variables
        for (int i = 0; i < nLocalVariables; ++i) { pop(); }

        Jr(31);
    }
    else throw runtime_error("procedure");
}
void codeMain(Node *n) {
    if (n->lhs != "main")
        throw runtime_error("not a main node");
    
    std::cout << ".import print\n";
    std::cout << ".import init\n";
    std::cout << ".import new\n";
    std::cout << ".import delete\n";

    // init
    // store 4 in $4
    Lis(4);
    Word(4);
    
    if (n->getChild("dcl", 1)->children[0]->children.size() == 2) { // INT STAR
        push(31);
        Lis(3);
        Word("init");
        Jalr(3);
        pop(31);
    }
    else { // INT
        push(31);
        push(2);
        Add(2, 0, 0);
        Lis(3);
        Word("init");
        Jalr(3);
        pop(2);
        pop(31);
    }
    



    map<string, int> offsetTable;
    int nLocalVariables = 0;

    {// dcl 1
    Node *dcl1 = n->getChild("dcl", 1);
    Node *dcl1Name = dcl1->getChild("ID", 1);
    offsetTable.insert(make_pair(dcl1Name->lexeme, 8));
    // push $1 to the stack
    push(1);}

    {// dcl 2
    Node *dcl2 = n->getChild("dcl", 2);
    Node *dcl2Name = dcl2->getChild("ID", 1);
    offsetTable.insert(make_pair(dcl2Name->lexeme, 4));
    // push $1 to the stack
    push(2);}

    Sub(29, 30, 4);

    // dcls
    Node *dcls = n->getChild("dcls", 1);
    int curOffset = 0;
    while (!(dcls->children.empty())) {
        
        string vName = dcls->getChild("dcl", 1)->getChild("ID", 1)->lexeme;
        string vValue = dcls->children[3]->lexeme;

        // store the variable to offsetTable
        offsetTable.insert(make_pair(vName, curOffset));

        // store the word to stack
        Lis(3);
        if (vValue == "NULL") {
            Word(1);
        } else {
            istringstream iss{vValue};
            int value;
            iss >> value;
            Word(value);
        }
        Sw(30, 3, -4);
        Sub(30, 30, 4);

        // recursion
        dcls = dcls->getChild("dcls", 1);
        curOffset -= 4;
        ++nLocalVariables;
    }

    std::cout << "; end of prelogue\n\n";

    // statements
    codeStatements(n->getChild("statements", 1), offsetTable);

    // return
    Node *expr = n->getChild("expr", 1);
    codeExpr(expr, offsetTable);
    // auto it = offsetTable.find(expr->children[0]->children[0]->children[0]->lexeme);
    // if (it != offsetTable.end()) Lw(29, 3, it->second);
    

    std::cout << "\n; begin of afterlogue\n";

    for (int i = 0; i < nLocalVariables; ++i) { pop(); }
    Jr(31);
}

int main() {

    Node *parseTree = new Node(cin);
    Node *procedures = parseTree->getChild("procedures", 1);
    try {
        // code for main
        Node *findMainProcedures = procedures;
        while (findMainProcedures) {
            if (findMainProcedures->rule == "procedures main") {
                codeMain(findMainProcedures->getChild("main", 1));
                break;
            }
            findMainProcedures = findMainProcedures->getChild("procedures", 1);
        }

        // code for procedures
        while (procedures->rule != "procedures main") {
            codeProcedure(procedures->getChild("procedure", 1));
            procedures = procedures->getChild("procedures", 1);
        }
    } 
    catch (runtime_error &e) {
        cerr << "ERROR: " << e.what() << endl;
    }

    // parseTree->print();
    delete parseTree;
}
