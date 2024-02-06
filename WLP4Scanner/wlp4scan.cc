#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

//// The dfa.h header declares a string constant containing a DFA file:
// std::string DFAstring = ...
//// The definition of the constant is in dfa.cc
#include "dfa.h"

//// Function that takes a DFA file (passed as a stream) and prints information about it.
void DFAprint(std::istream &in);

//// These helper functions are defined at the bottom of the file:
// Check if a string is a single character.
bool isChar(std::string s);
// Check if a string represents a character range.
bool isRange(std::string s);
// Remove leading and trailing whitespace from a string, and
// squish intermediate whitespace sequences down to one space each.
std::string squish(std::string s);
// Convert hex digit character to corresponding number.
int hexToNum(char c);
// Convert number to corresponding hex digit character.
char numToHex(int d);
// Replace all escape sequences in a string with corresponding characters.
std::string escape(std::string s);
// Convert non-printing characters or spaces in a string into escape sequences.
std::string unescape(std::string s);

const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";

using namespace std;

struct State {
    string name;
    bool accepting;
    State (string name, bool accepting) : name{name}, accepting{accepting} {}
};

struct Transition {
    string fromState;
    string toState;
    char transition;
    Transition (string fromState, string toState, char transition) :
        fromState{fromState}, toState{toState}, transition{transition} {}
    bool validTransition (string curState, char trans) {
        if (this->fromState == curState && this->transition == trans)  return true;
        return false;
    }
};

struct DFA {
    // starting state is states[0]
    vector<State> states;
    vector<Transition> transitions;
    DFA () {}
};

struct Token {
    string kind;
    string lexeme;
    Token (string kind, string lexeme) : kind{kind}, lexeme{lexeme} {}
};

// struct Node {
//     string data;
//     vector<Node *> children;
//     Node(string data): data{data} {}
//     ~Node() {
//         for ( auto &c : children ) { delete c; }
//     }
//     void print(string prefix, ostream &out) {
//         out << data << endl;
//         for (auto &c: children) {
//             if (c != children.back()) {
//                 cout << prefix << JOINT;
//                 c->print(prefix + BAR, out);
//             }
//             else {
//                 cout << prefix << CORNER;
//                 c->print(prefix + SPACER, out);
//             }
//         }
//     }
// };

void readDFA(istream &in, DFA &dfa);
vector<Token> smm(istream &in, DFA &dfa);
bool getNextState (State &curState, const char trans, const DFA &dfa);
bool validNum (const string t);
bool needConvertToId (const string kind);

int main() {
  try {
    DFA dfa{};
    std::stringstream s(DFAstring);
    readDFA(s, dfa);
    vector<Token> tokenLexemes = smm(cin, dfa);
    for ( Token tk : tokenLexemes ) {
        cout << tk.kind << " " << tk.lexeme << endl;
    }
  } catch(std::runtime_error &e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
  return 0;
}

void readDFA(istream &in, DFA &dfa) {
    string s;
    // Skip blank lines at the start of the file
    while (true) {
        if(!(getline(in, s))) {
            throw runtime_error
                ("Expected " + STATES + ", but found end of input.");
        }
        s = squish(s);
        if (s == STATES) {
            break;
        }
        if (!s.empty()) {
            throw std::runtime_error
                ("Expected " + STATES + ", but found: " + s);
        }
    }

    // Store states
    while (true) {
        if (!(in >> s)) {
            throw std::runtime_error
                ("Unexpected end of input while reading state set: " 
                + TRANSITIONS + "not found.");
        }
        if (s == TRANSITIONS) {
            break;
        }
        // Process an individual state
        bool accepting = false;
        if (s.back() == '!' && s.length() > 1) {
            accepting = true;
            s.pop_back();
        }
        // Store the state
        dfa.states.push_back({s, accepting});
    }

    // Store transitions
    getline(in, s); // Skip .TRANSITIONS header
    while (true) {

        if (!getline(in, s)) {
            // EOF
            break;
        }
        s = squish(s);
        if (s == INPUT) {
            break;
        }
        
        // Split the line into parts
        string lineStr = s;
        istringstream line(lineStr);
        vector<string> lineVec;
        while (line >> s) {
            lineVec.push_back(s);
        }
        if (lineVec.empty()) {
            // Skip blank lines
            continue;
        }
        if (lineVec.size() < 3) {
            throw runtime_error
                ("Incomplete transition line: " + lineStr);
        }
        // Extract state information from the line
        string fromState = lineVec.front();
        string toState = lineVec.back();
        // Extract character and range information from the line
        vector<char> charVec;
        for (int i = 1; i < lineVec.size()-1; ++i) {
            string charOrRange = escape(lineVec[i]);
            if (isChar(charOrRange)) {
                char c = charOrRange[0];
                if (c < 0 || c > 127) {
                    throw runtime_error
                        ("Invalid (non-ASCII) character in transition line: " + lineStr + "\n"
                         + "Character " + unescape(std::string(1,c)) + " is outside ASCII range");

                }
                charVec.push_back(c);
            }
            else if (isRange(charOrRange)) {
                for (char c = charOrRange[0]; charOrRange[0] <= c && c <= charOrRange[2]; ++c) {
                    charVec.push_back(c);
                }
            }
            else {
                throw std::runtime_error
                    ("Expected character or range, but found "
                    + charOrRange + " in transition line: " + lineStr);
            }
        }
        // Store these transitions into DFA
        for ( char c : charVec ) {
            dfa.transitions.push_back({fromState, toState, c});
        }
    }
    // We ignore .INPUT sections, so we are done
}

vector<Token> smm(istream &in, DFA &dfa) {
    vector<Token> tokenLexemes;
    State p = dfa.states[0];
    string t = "";
    in.peek();
    while (!(in.eof())) {
        char a = in.peek();
        if (getNextState(p, a, dfa)) {
            t += in.get();
        }
        else {
            // stuck
            if (p.accepting) {
                // Check the restrictions
                if (p.name == "NUM") {
                    if (!validNum(t)) {
                        throw runtime_error ("NUM value is not valid");
                    }
                }
                if (p.name[0] != '?') {
                    if (needConvertToId(p.name)) tokenLexemes.push_back({"ID", t});
                    else tokenLexemes.push_back({p.name, t});
                }
                p = dfa.states[0];
                t = "";
            }
            else {
                throw runtime_error ("Scanning Failure");
            }
        }
    }
    if (p.name == "start") {
        return tokenLexemes;
    }
    if (p.accepting) {
        if (p.name[0] != '?') {
            if (needConvertToId(p.name)) tokenLexemes.push_back({"ID", t});
            else tokenLexemes.push_back({p.name, t});
        }
        return tokenLexemes;
    }
    else {
        throw runtime_error ("Scanning Failure");
    }
    return tokenLexemes;
}

//// Helper functions

bool isChar(std::string s) {
  return s.length() == 1;
}

bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}

std::string squish(std::string s) {
  std::stringstream ss(s);
  std::string token;
  std::string result;
  std::string space = "";
  while(ss >> token) {
    result += space;
    result += token;
    space = " ";
  }
  return result;
}

int hexToNum(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return 10 + (c - 'a');
  } else if ('A' <= c && c <= 'F') {
    return 10 + (c - 'A');
  }
  // This should never happen....
  throw std::runtime_error("Invalid hex digit!");
}

char numToHex(int d) {
  return (d < 10 ? d + '0' : d - 10 + 'A');
}

std::string escape(std::string s) {
  std::string p;
  for(int i=0; i<s.length(); ++i) {
    if (s[i] == '\\' && i+1 < s.length()) {
      char c = s[i+1]; 
      i = i+1;
      if (c == 's') {
        p += ' ';            
      } else
      if (c == 'n') {
        p += '\n';            
      } else
      if (c == 'r') {
        p += '\r';            
      } else
      if (c == 't') {
        p += '\t';            
      } else
      if (c == 'x') {
        if(i+2 < s.length() && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
          if (hexToNum(s[i+1]) > 8) {
            throw std::runtime_error(
                "Invalid escape sequence \\x"
                + std::string(1, s[i+1])
                + std::string(1, s[i+2])
                +": not in ASCII range (0x00 to 0x7F)");
          }
          char code = hexToNum(s[i+1])*16 + hexToNum(s[i+2]);
          p += code;
          i = i+2;
        } else {
          p += c;
        }
      } else
      if (isgraph(c)) {
        p += c;            
      } else {
        p += s[i];
      }
    } else {
       p += s[i];
    }
  }  
  return p;
}

std::string unescape(std::string s) {
  std::string p;
  for(int i=0; i<s.length(); ++i) {
    char c = s[i];
    if (c == ' ') {
      p += "\\s";
    } else
    if (c == '\n') {
      p += "\\n";
    } else
    if (c == '\r') {
      p += "\\r";
    } else
    if (c == '\t') {
      p += "\\t";
    } else
    if (!isgraph(c)) {
      std::string hex = "\\x";
      p += hex + numToHex((unsigned char)c/16) + numToHex((unsigned char)c%16);
    } else {
      p += c;
    }
  }
  return p;
}

// return true if found the next state, false otherwise
// if true, modify the curState to be the next State
bool getNextState (State &curState, const char trans, const DFA &dfa) {
    for ( Transition t : dfa.transitions ) {
        if (t.validTransition(curState.name, trans)) {
            curState.name = t.toState;
            for ( State s : dfa.states ) {
                if (s.name == curState.name) {
                    curState.accepting = s.accepting;
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

// check if the numeric value does not exceed 2147483647
bool validNum (const string t) {
    int len = t.length();
    // should not have starting 0s if the value is not 0
    if (len > 1 && t[0] == '0')  return false;
    // len > 10 means the numeric value exceeds INT_MAX
    if (len > 10)  return false;
    istringstream iss{t};
    long int x = 0;
    iss >> x;
    if (x > 2147483647)  return false;
    return true;
}

bool needConvertToId (const string kind) {
    if (kind == "firsti" ||
        kind == "intn" ||
        kind == "firstw" ||
        kind == "waina" ||
        kind == "waini" ||
        kind == "elsee" ||
        kind == "elsel" ||
        kind == "elses" ||
        kind == "whileh" ||
        kind == "whilei" ||
        kind == "whilel" ||
        kind == "printlnp" ||
        kind == "printlnr" ||
        kind == "printlni" ||
        kind == "printlnn" ||
        kind == "printlnt" ||
        kind == "printlnl" ||
        kind == "returnr" ||
        kind == "returne" ||
        kind == "returnt" ||
        kind == "returnu" ||
        kind == "returnr2" ||
        kind == "newn" ||
        kind == "newe" ||
        kind == "deleted" ||
        kind == "deletee" ||
        kind == "deletel" ||
        kind == "deletee2" ||
        kind == "deletet" ||
        kind == "nullN" ||
        kind == "nullU" ||
        kind == "nullL")  return true;
    return false;
}
