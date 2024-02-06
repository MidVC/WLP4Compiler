#include "dfa.h"

std::string DFAstring = R"(
.STATES
start
ID!
NUM!
INT!
WAIN!
IF!
ELSE!
WHILE!
PRINTLN!
RETURN!
NEW!
DELETE!
NULL!
LPAREN!
RPAREN!
LBRACE!
RBRACE!
LBRACK!
RBRACK!
BECOMES!
PLUS!
MINUS!
STAR!
SLASH!
PCT!
AMP!
COMMA!
SEMI!
LT!
GT!
LE!
GE!
EQ!
NE!
?WHITESPACE!
?COMMENT!
firsti!
intn!
firstw!
waina!
waini!
elsee!
elsel!
elses!
whileh!
whilei!
whilel!
printlnp!
printlnr!
printlni!
printlnn!
printlnt!
printlnl!
returnr!
returne!
returnt!
returnu!
returnr2!
newn!
newe!
deleted!
deletee!
deletel!
deletee2!
deletet!
nullN!
nullU!
nullL!
exclamation
.TRANSITIONS
start 0-9 NUM
NUM 0-9 NUM
start i firsti
firsti a-e g-m o-z A-Z 0-9 ID
firsti n intn
intn a-s u-z A-Z 0-9 ID
intn t INT
INT a-z A-Z 0-9 ID
start w firstw
firstw b-g i-z A-Z 0-9 ID
firstw a waina
waina a-h j-z A-Z 0-9 ID
waina i waini
waini a-m o-z A-Z 0-9 ID
waini n WAIN
WAIN a-z A-Z 0-9 ID
firsti f IF
IF a-z A-Z 0-9 ID
start e elsee
elsee a-k m-z A-Z 0-9 ID
elsee l elsel
elsel a-r t-z A-Z 0-9 ID
elsel s elses
elses a-d f-z A-Z 0-9 ID
elses e ELSE
ELSE a-z A-Z 0-9 ID
firstw h whileh
whileh a-h j-z A-Z 0-9 ID
whileh i whilei
whilei a-k m-z A-Z 0-9 ID
whilei l whilel
whilel a-d f-z A-Z 0-9 ID
whilel e WHILE
WHILE a-z A-Z 0-9 ID
start p printlnp
printlnp a-q s-z A-Z 0-9 ID
printlnp r printlnr
printlnr a-h j-z A-Z 0-9 ID
printlnr i printlni
printlni a-m o-z A-Z 0-9 ID
printlni n printlnn
printlnn a-s u-z A-Z 0-9 ID
printlnn t printlnt
printlnt a-k m-z A-Z 0-9 ID
printlnt l printlnl
printlnl a-m o-z A-Z 0-9 ID
printlnl n PRINTLN
PRINTLN a-z A-Z 0-9 ID
start r returnr
returnr a-d f-z A-Z 0-9 ID
returnr e returne
returne a-s u-z A-Z 0-9 ID
returne t returnt
returnt a-t v-z A-Z 0-9 ID
returnt u returnu
returnu a-q s-t A-Z 0-9 ID
returnu r returnr2
returnr2 a-m o-z A-Z 0-9 ID
returnr2 n RETURN
RETURN a-z A-Z 0-9 ID
start n newn
newn a-d f-z A-Z 0-9 ID
newn e newe
newe a-v x-z A-Z 0-9 ID
newe w NEW
NEW a-z A-Z 0-9 ID
start d deleted
deleted a-d f-z A-Z 0-9 ID
deleted e deletee
deletee a-k m-z A-Z 0-9 ID
deletee l deletel
deletel a-d f-z A-Z 0-9 ID
deletel e deletee2
deletee2 a-s u-z A-Z 0-9 ID
deletee2 t deletet
deletet a-d f-z A-Z 0-9 ID
deletet e DELETE
DELETE a-z A-Z 0-9 ID
start N nullN
nullN a-z A-T V-Z 0-9 ID
nullN U nullU
nullU a-z A-K M-Z 0-9 ID
nullU L nullL
nullL a-z A-K M-Z 0-9 ID
nullL L NULL
NULL a-z A-Z 0-9 ID
start a-c f-h j-m o q s-v x-z A-M O-Z ID
ID a-z A-Z 0-9 ID
start ( LPAREN
start ) RPAREN
start { LBRACE
start } RBRACE
start [ LBRACK
start ] RBRACK
start = BECOMES
start + PLUS
start - MINUS
start * STAR
start / SLASH
start % PCT
start & AMP
start , COMMA
start ; SEMI
start < LT
start > GT
LT = LE
GT = GE
BECOMES = EQ
start ! exclamation
exclamation = NE
start \x20 \x09 \x0a \x0d ?WHITESPACE
?WHITESPACE \x20 \x09 \x0a \x0d ?WHITESPACE
SLASH / ?COMMENT
?COMMENT \x00-\x09 \x0B \x0C \x0E-\x7F ?COMMENT
)";
