#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
using namespace std;

const string EMPTY = ".EMPTY";

enum class TypeVariable { INT, PINT, NOTYPE };
ostream &operator<<(ostream &out, const TypeVariable &tv) {
    switch (tv) {
        case (TypeVariable::INT):
            out << ": INT";
            break;
        case (TypeVariable::PINT):
            out << ": PINT";
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
        }
        else { // line is a nonterminal node
            rule = line;
            iss >> lhs;
            string rhs;
            while (iss >> rhs) {
                if (rhs != EMPTY) {
                    Node *child = new Node(in);
                    children.push_back(child);
                }
            }
        }
    }
    void print() {
        cout << rule << kind << ' ' << lexeme
             << type << endl;
        for ( auto &n : children ) { n->print(); }
    }
    ~Node() {
        for ( auto &n : children ) { delete n; }
    }
};

// Node have lhs "type"
TypeVariable convertType(const Node *n) {
    if (n->children.size() == 1) return TypeVariable::INT;
    else return TypeVariable::PINT;
}

struct Variable {
    string name;
    TypeVariable type;
    Variable(const Node *dcl) {
        if (dcl->lhs != "dcl")
            throw runtime_error("lhs is not dcl");
        name = dcl->children[1]->lexeme;
        type = convertType(dcl->children[0]);
    }
};

struct VariableTable {
    map<string, Variable> table;
    void add(const Variable v) {
        auto iterator = table.find(v.name);
        if (iterator != table.end())
            throw runtime_error("duplicate variable declaration");
        table.insert(make_pair(v.name, v));
    }
    Variable get(const string s) {
        auto iterator = table.find(s);
        if (iterator == table.end())
            throw runtime_error("use of undeclared variable");
        return iterator->second;
    }
};

void readParams(const Node *n, vector<TypeVariable> &s, VariableTable &t) {
    if (n->lhs == "dcl") {
        s.push_back(convertType(n->children[0]));
        t.add(Variable(n));
    }
    else {
        for ( auto &nn : n->children ) {
            readParams(nn, s, t);
        }
    }
}

void readDcls(const Node *n, VariableTable &t) {
    if (n->lhs == "dcl") {
        t.add(Variable(n));
    }
    else {
        for ( auto &nn : n->children ) {
            readDcls(nn, t);
        }
    }
}

struct Procedure {
    string name;
    vector<TypeVariable> signature;
    VariableTable symbolTable;
    Procedure(const Node *n) {
        if (n->children[0]->kind != "INT")
            throw runtime_error("procedure/wain not returning an int");
        if (n->lhs == "procedure") {
            name = n->children[1]->lexeme;
            // read params tree
            Node *params = n->children[3];
            readParams(params, signature, symbolTable);
            // read dcls tree
            Node *dcls = n->children[6];
            readDcls(dcls, symbolTable);
        }
        else if (n->lhs == "main") {
            name = "wain";
            signature.push_back(convertType(n->children[3]->children[0]));
            TypeVariable wain2nd = convertType(n->children[5]->children[0]);
            if (wain2nd != TypeVariable::INT)
                throw runtime_error("second param of wain is not an int");
            signature.push_back(wain2nd);
            symbolTable.add(Variable(n->children[3]));
            symbolTable.add(Variable(n->children[5]));
            Node *dcls = n->children[8];
            readDcls(dcls, symbolTable);
        }
        else throw runtime_error("not a procedure or main");
    }
};

struct ProcedureTable {
    map<string, Procedure> table;
    void add(const Procedure p) {
        auto iterator = table.find(p.name);
        if (iterator != table.end())
            throw runtime_error("duplicate procedure declaration");
        table.insert(make_pair(p.name, p));
    }
    Procedure get(const string s) {
        auto iterator = table.find(s);
        if (iterator == table.end())
            throw runtime_error("use of undeclared procedure");
        return iterator->second;
    }
};

void annotateExpr(Node *tree, ProcedureTable &pt, Procedure &p) {
    if (tree->rule == "expr term") tree->type = tree->children[0]->type;
    else if (tree->rule == "expr expr PLUS term") {
        if (tree->children[0]->type == TypeVariable::INT &&
            tree->children[2]->type == TypeVariable::INT)
            tree->type = TypeVariable::INT;
        else if (tree->children[0]->type == TypeVariable::PINT &&
                 tree->children[2]->type == TypeVariable::INT)
            tree->type = TypeVariable::PINT;
        else if (tree->children[0]->type == TypeVariable::INT &&
                 tree->children[2]->type == TypeVariable::PINT)
            tree->type = TypeVariable::PINT;
        else throw runtime_error(tree->rule + ", bad PLUS");
    }
    else {
        if (tree->children[0]->type == TypeVariable::INT &&
            tree->children[2]->type == TypeVariable::INT)
            tree->type = TypeVariable::INT;
        else if (tree->children[0]->type == TypeVariable::PINT &&
                 tree->children[2]->type == TypeVariable::INT)
            tree->type = TypeVariable::PINT;
        else if (tree->children[0]->type == TypeVariable::PINT &&
                 tree->children[2]->type == TypeVariable::PINT)
            tree->type = TypeVariable::INT;
        else throw runtime_error(tree->rule + ", bad MINUS");
    }
}

void annotateTerm(Node *tree, ProcedureTable &pt, Procedure &p) {
    if (tree->rule == "term factor") tree->type = tree->children[0]->type;
    else {
        if (tree->children[0]->type != TypeVariable::INT ||
            tree->children[2]->type != TypeVariable::INT)
            throw runtime_error(tree->rule + ", term or factor not of type INT");
        tree->type = TypeVariable::INT;
    }
}

void annotateFactor(Node *tree, ProcedureTable &pt, Procedure &p) {
    if (tree->rule == "factor NUM") tree->type = TypeVariable::INT;
    else if (tree->rule == "factor NULL") tree->type = TypeVariable::PINT;
    else if (tree->rule == "factor ID") {
        Variable v = p.symbolTable.get(tree->children[0]->lexeme);
        tree->type = v.type;
    } else if (tree->rule == "factor LPAREN expr RPAREN") tree->type = tree->children[1]->type;
    else if (tree->rule == "factor AMP lvalue") {
        if (tree->children[1]->type != TypeVariable::INT)
            throw runtime_error("factor AMP lvalue, lvalue is not of type INT");
        tree->type = TypeVariable::PINT;
    } else if (tree->rule == "factor STAR factor") {
        if (tree->children[1]->type != TypeVariable::PINT)
            throw runtime_error("factor STAR factor, second factor is not of type PINT");
        tree->type = TypeVariable::INT;
    } else if (tree->rule == "factor NEW INT LBRACK expr RBRACK") {
        if (tree->children[3]->type != TypeVariable::INT)
            throw runtime_error(tree->rule + ", expr is not of type INT");
        tree->type = TypeVariable::PINT;
    } else if (tree->rule == "factor ID LPAREN RPAREN") {
        string id = tree->children[0]->lexeme;
        auto iterator = p.symbolTable.table.find(id);
        if (iterator != p.symbolTable.table.end())
            throw runtime_error(id + " is in the local symbol table");
        auto procedure = pt.get(id);
        if (!procedure.signature.empty())
            throw runtime_error("why are there parameters when calling " + id);
        tree->type = TypeVariable::INT;
    } else if (tree->rule == "factor ID LPAREN arglist RPAREN") {
        string id = tree->children[0]->lexeme;
        auto iterator = p.symbolTable.table.find(id);
        if (iterator != p.symbolTable.table.end())
            throw runtime_error(id + " is in the local symbol table");
        auto procedure = pt.get(id);
        vector<TypeVariable> paramTypes;
        Node *arglist = tree->children[2];
        while (true) {
            paramTypes.push_back(arglist->children[0]->type);
            if (arglist->children.size() == 1) break;
            arglist = arglist->children[2];
        }
        if (paramTypes != procedure.signature)
            throw runtime_error("invalid parameters passed when calling " + id);
        tree->type = TypeVariable::INT;
    }
}

void annotateLvalue(Node *tree, ProcedureTable &pt, Procedure &p) {
    if (tree->rule == "lvalue ID") {
        Variable v = p.symbolTable.get(tree->children[0]->lexeme);
        tree->type = v.type;
    }
    else if (tree->rule == "lvalue LPAREN lvalue RPAREN") tree->type = tree->children[1]->type;
    else if (tree->rule == "lvalue STAR factor") {
        if (tree->children[1]->type != TypeVariable::PINT)
            throw runtime_error("lvalue STAR factor, factor is not of type PINT");
        tree->type = TypeVariable::INT;
    }
}

void annotateTypes(Node *tree, ProcedureTable &pt, Procedure &p) {
    for ( auto &n : tree->children ) {
        annotateTypes(n, pt, p);
    }
    if (tree->lhs == "expr") annotateExpr(tree, pt, p);
    else if (tree->lhs == "term") annotateTerm(tree, pt, p);
    else if (tree->lhs == "factor") annotateFactor(tree, pt, p);
    else if (tree->lhs == "lvalue") annotateLvalue(tree, pt, p);
}

void checkOtherRequirements(Node *tree) {
    for ( auto &n : tree->children ) { checkOtherRequirements(n); }
    if (tree->rule == "statement lvalue BECOMES expr SEMI") {
        if (tree->children[0]->type != tree->children[2]->type)
            throw runtime_error(tree->rule + ", lvalue is not the same type as expr");
    } else if (tree->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        if (tree->children[2]->type != TypeVariable::INT)
            throw runtime_error(tree->rule + ", expr not an INT");
    } else if (tree->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        if (tree->children[3]->type != TypeVariable::PINT)
            throw runtime_error(tree->rule + ", expr not an PINT");
    } else if (tree->lhs == "test") {
        if (tree->children[0]->type != tree->children[2]->type)
            throw runtime_error(tree->rule + ", two expr are not of same type");
    } else if (tree->rule == "dcls dcls dcl BECOMES NUM SEMI") {
        if (convertType(tree->children[1]->children[0]) != TypeVariable::INT)
            throw runtime_error(tree->rule + ", dcl does not declare an INT");
    } else if (tree->rule == "dcls dcls dcl BECOMES NULL SEMI") {
        if (convertType(tree->children[1]->children[0]) != TypeVariable::PINT)
            throw runtime_error(tree->rule + ", dcl does not declare a PINT");
    } else if (tree->lhs == "procedure") {
        if (tree->children[9]->type != TypeVariable::INT)
            throw runtime_error("procedure " + tree->children[1]->lexeme + " does not return an INT");
    } else if (tree->lhs == "main") {
        if (tree->children[11]->type != TypeVariable::INT)
            throw runtime_error("wain function does not return an INT");
    }
}

void collectProcedures(Node *tree, ProcedureTable &p) {
    if (tree->lhs == "procedure" || tree->lhs == "main") {
        Procedure pcd{tree};
        p.add(pcd);
        annotateTypes(tree, p, pcd);
    }
    else for ( auto &t : tree->children ) {
        collectProcedures(t, p);
    }
}

int main() {
    Node *parseTree = new Node(cin);
    try {
        ProcedureTable ptable;
        collectProcedures(parseTree->children[1], ptable);
        checkOtherRequirements(parseTree->children[1]);
        parseTree->print();
    } catch (runtime_error &e) {
        cerr << "ERROR: " << e.what() << endl;
    }
    delete parseTree;
}
