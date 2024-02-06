#include "wlp4data.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <iostream>
using namespace std;


const string DERIVATION = ".DERIVATION";
const string EMPTY = ".EMPTY";
const string JOINT = "|-";
const string BAR = "| ";
const string CORNER = "'-";
const string SPACER = "  ";

struct Rule {
    string lhs;
    vector<string> rhs;
    Rule(string lhs, vector<string> rhs): lhs{lhs}, rhs{rhs} {};
    string getString() const {
        string result = lhs;
        for ( auto &n : rhs ) { result += ' ' + n; }
        if (rhs.size() == 0) result += ' ' + EMPTY;
        return result;
    }
    int rhsSize() const { return rhs.size(); }
};

struct DFA {
    map<pair<int, string>, int> transitions;
    map<pair<int, string>, int> reductions;
    void insertTransition(int startState, string symbol, int endState) {
        transitions.insert(make_pair(make_pair(startState, symbol), endState));
    }
    void insertReduction(int startState, string symbol, int ruleNumber) {
        reductions.insert(make_pair(make_pair(startState, symbol), ruleNumber));
    }
    // search for a transition.
    // return the state index if found a transition, -1 otherwise
    int findTransition(const int state, const string symbol) const {
        int resultState = -1;
        pair<int, string> key = make_pair(state, symbol);
        auto iterator = transitions.find(key);
        if (iterator != transitions.end()) {
            resultState = iterator->second;
        }
        return resultState;
    }
    // search for a reduction
    // return the rule index if found a rule, -1 otherwise
    int findReduction(const int state, const string symbol) const {
        int ruleIndex = -1;
        pair<int, string> key = make_pair(state, symbol);
        auto iterator = reductions.find(key);
        if (iterator != reductions.end()) {
            ruleIndex = iterator->second;
        }
        return ruleIndex;
    }
};

struct Token {
    string kind;
    string lexeme;
    Token (string kind, string lexeme) : kind{kind}, lexeme{lexeme} {}
    string getString() const { return kind + ' ' + lexeme; }
};

struct Node {
    string data;
    vector<Node *> children;

    Node(string data): data{data} {}

    ~Node() {
        for ( auto &c : children ) { delete c; }
    }

    void addChild(Node *child) { children.push_back(child); }

    void print(ostream &out) {
        out << data << endl;
        for (auto &c: children) { c->print(out); }
    }
};

void readCFG(const string &in, vector<Rule> &cfg) {
    string line;
    string word;
    istringstream iss{in};
    getline(iss, line); // skip the .CFG line
    while (getline(iss, line)) {
        istringstream issLine{line};
        string lhs;
        vector<string> rhs;
        issLine >> lhs;
        while (issLine >> word) {
            if (word != EMPTY) rhs.push_back(word);
            else break;
        }
        cfg.push_back(Rule(lhs, rhs));
    }
}

void readTransitions(const string &in, DFA &dfa) {
    istringstream iss{in};
    string line;
    int startState;
    string symbol;
    int endState;
    getline(iss, line); // skip the .TRANSITIONS line
    while (getline(iss, line)) {
        istringstream issLine{line};
        issLine >> startState >> symbol >> endState;
        dfa.insertTransition(startState, symbol, endState);
    }
}

void readReductions(const string &in, DFA &dfa) {
    istringstream iss{in};
    string line;
    int startState;
    string symbol;
    int ruleNumber;
    getline(iss, line); // skip the .REDUCTIONS line
    while (getline(iss, line)) {
        istringstream issLine{line};
        issLine >> startState >> ruleNumber >> symbol;
        dfa.insertReduction(startState, symbol, ruleNumber);
    }
}

// read tokens from stdin (cin)
void readTokens(vector<Token> &tokens) {
    string line;
    string kind = "BOF";
    string lexeme = "BOF";
    // Add BOF BOF
    tokens.push_back(Token(kind, lexeme));
    while (getline(cin, line)) {
        istringstream iss{line};
        iss >> kind >> lexeme;
        tokens.push_back(Token(kind, lexeme));
    }
    // Add EOF EOF
    kind = "EOF";
    lexeme = "EOF";
    tokens.push_back(Token(kind, lexeme));
}

// according to the rule, pop the rhs trees and then
// push the new lhs tree to the treeStack
void reduceTrees(const Rule &rule, vector<Node *> &treeStack) {
    Node *newTree = new Node(rule.getString());
    int len = rule.rhsSize();
    int lenStack = treeStack.size();
    // add the tree nodes to be the children of the new tree
    for (int i = lenStack - len; i < lenStack; ++i) {
        newTree->addChild(treeStack[i]);
    }
    // pop those children trees from treeStack
    for (int i = 0; i < len; ++i) { treeStack.pop_back(); }
    // push the new tree to the treeStack
    treeStack.push_back(newTree);
}

// according to the rule, pop the states at the rhs of the rule and
// push the new state to be the lhs of the rule
void reduceStates(const Rule &rule, vector<int> &stateStack, const DFA &dfa) {
    int len = rule.rhsSize();
    // pop the last len states from the state stack
    for (int i = 0; i < len; ++i) { stateStack.pop_back(); }
    int newState = dfa.findTransition(stateStack.back(), rule.lhs);
    stateStack.push_back(newState);
}

// shift, given the Token
void shift(vector<Node *> &treeStack, vector<int> &stateStack, const DFA &dfa, const Token &tk) {
    Node *newTree = new Node(tk.getString());
    treeStack.push_back(newTree);
    int nextState = dfa.findTransition(stateStack.back(), tk.kind);
    if (nextState == -1) {
        throw runtime_error("Can not find the transition!");
    }
    else stateStack.push_back(nextState);
}


int main() {
    vector<Rule> cfg;
    DFA dfa;
    vector<Token> tokens;
    vector<Node *> treeStack;
    vector<int> stateStack;
    try {
        // initialization
        readCFG(WLP4_CFG, cfg);
        readTransitions(WLP4_TRANSITIONS, dfa);
        readReductions(WLP4_REDUCTIONS, dfa);
        readTokens(tokens);
        stateStack.push_back(0);

        // parsing loop
        int index = 0;
        int len = tokens.size();
        while (index < len) {
            int ruleIndex = dfa.findReduction(stateStack.back(), tokens[index].kind);
            while (ruleIndex != -1) {
                reduceTrees(cfg[ruleIndex], treeStack);
                reduceStates(cfg[ruleIndex], stateStack, dfa);
                ruleIndex = dfa.findReduction(stateStack.back(), tokens[index].kind);
            }
            shift(treeStack, stateStack, dfa, tokens[index]);
            index += 1;
        }
        // obtain the final tree
        reduceTrees(cfg[0], treeStack);
        reduceStates(cfg[0], stateStack, dfa);

        // print parse tree
        if (treeStack.size() != 1) {
            throw runtime_error("idk what happened");
        }
        treeStack[0]->print(cout);
    } catch (runtime_error &e) {
        cerr << "ERROR: " << e.what() << endl;
    }
    for ( auto &n : treeStack ) {
        delete n;
    }
}
