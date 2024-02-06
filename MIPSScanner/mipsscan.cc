#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "dfa.h"
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

const string STATES      = ".STATES";
const string TRANSITIONS = ".TRANSITIONS";
const string INPUT       = ".INPUT";

//// Helper functions copied from starter code

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

// check if string t is below hex value 0xFFFFFFFF
bool validHexInt (const string t) {
    int len = t.length();
    int indexNotZero = len;
    // Find the first heximal value that is not 0 after 0x
    for (int i = 2; i < len; ++i) {
        if (t[i] != '0' ) {
            indexNotZero = i;
            break;
        }
    }
    if ((len - indexNotZero) > 8)  return false;
    return true;
}

// check if string t is within -2147483648 and 4294967295
bool validDecInt (const string t) {
    int len = t.length();
    int indexNotZero = len;
    // Find the first decimal value that is not 0
    for (int i = 0; i < len; ++i) {
        if ((t[i] != '0') && (t[i] != '-')) {
            indexNotZero = i;
            break;
        }
    }
    if ((len - indexNotZero) > 10)  return false;
    // Copy the num value without the 0s
    string s = "";
    if (t[0] == '-') {
        s.push_back('-');
    }
    for (int i = indexNotZero; i < len; ++i) {
        s.push_back(t[i]);
    }
    // Read the value x
    istringstream iss{s};
    long int x = 0;
    iss >> x;
    if (x < -2147483648 || x > 4294967295)  return false;
    return true;
}

// check if t is a valid register
bool validRegister (const string t) {
    int len = t.length();
    int indexNotZero = len;
    for (int i = 1; i < len; ++i) {
        if (t[i] != '0') {
            indexNotZero = i;
            break;
        }
    }
    if ((len - indexNotZero) > 2)  return false;
    string s = "";
    for (int i = indexNotZero; i < len; ++i) {
        s.push_back(t[i]);
    }
    istringstream iss{s};
    int x = 0;
    iss >> x;
    if (x < 0 || x > 31)  return false;
    return true;
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
                if (p.name == "REGISTER") {
                    if (!validRegister(t)) {
                        throw runtime_error ("ERROR: Register value exceeds 31");
                    }
                }
                if (p.name == "DECINT") {
                    if (!validDecInt(t)) {
                        throw runtime_error
                            ("ERROR: Decimal value is beyond the union of signed and unsigned ranges of 32-bit integers");
                    }
                }
                if (p.name == "HEXINT") {
                    if (!validHexInt(t)) {
                        throw runtime_error
                            ("ERROR: Heximal value is greater than 0xFFFFFFFF");
                    }
                }
                if (p.name[0] != '?') {
                    if (p.name == "ZERO") {
                        tokenLexemes.push_back({"DECINT", t});
                    }
                    else {
                        tokenLexemes.push_back({p.name, t});
                    }
                }
                p = dfa.states[0];
                t = "";
            }
            else {
                throw runtime_error ("ERROR: Scanning Failure");
            }
        }
    }
    if (p.name == "start") {
        return tokenLexemes;
    }
    if (p.accepting) {
        if (p.name[0] != '?') {
            tokenLexemes.push_back({p.name, t});
        }
        return tokenLexemes;
    }
    else {
        throw runtime_error ("ERROR: Scanning Failure");
    }
    return tokenLexemes;
}

void printDFA(DFA &dfa) {
    // Print states
    cout << "States:" << endl;
    bool initial = true;
    for ( State s : dfa.states ) {
        cout << s.name
             << (initial ? " (initial)" : "")
             << (s.accepting ? " (accepting)" : "")
             << endl;
        initial = false;
    }
    
    // Print transition
    cout << "Transitions:" << endl;
    for ( Transition t : dfa.transitions ) {
        cout << t.fromState
             << " " << unescape(string(1, t.transition))
             << " " << t.toState << endl;
    }
}

int main() {
    
    // Read the DFA
    DFA dfa{};
    try {
        istringstream iss{DFAstring};
        readDFA(iss, dfa);
    } catch (runtime_error &e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }

    // Read the input to generate tokens
    vector<Token> tokenLexemes;
    try {
        tokenLexemes = smm(cin, dfa);
    } catch (runtime_error &e) {
        cerr << e.what() << endl;
        return 1;
    }

    // Print the tokens
    for ( Token tk : tokenLexemes ) {
        cout << tk.kind
             << ((tk.kind == "NEWLINE") ? "" : (" " + tk.lexeme))
             << endl;
    }

    return 0;

}
