#include <bits/stdc++.h>
#include "iostream"
using namespace std;

// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=strlen(b);
    return strncmp(a, b, nb)==0;
}

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
    if(b==0) {*a=0; return;}
    int n=strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
    FILE* file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char* str) {file=0; if(str) file=fopen(str, "r"); cur_line_size=0; cur_ind=0; cur_line_num=0;}
    ~InFile(){if(file) fclose(file);}

    void SkipSpaces()
    {
        while(cur_ind<cur_line_size)
        {
            char ch=line_buf[cur_ind];
            if(ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char* str)
    {
        while(true)
        {
            SkipSpaces();
            while(cur_ind>=cur_line_size) {if(!GetNewLine()) return false; SkipSpaces();}

            if(StartsWith(&line_buf[cur_ind], str))
            {
                cur_ind+=strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=strlen(line_buf);
        if(cur_line_size==0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char* GetNextTokenStr()
    {
        SkipSpaces();
        while(cur_ind>=cur_line_size) {if(!GetNewLine()) return 0; SkipSpaces();}
        return &line_buf[cur_ind];
    }

    void Advance(int num)
    {
        cur_ind+=num;
    }
};

struct OutFile
{
    FILE* file;
    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
            : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType{
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    ASSIGN, EQUAL, LESS_THAN,
    PLUS, MINUS, TIMES, DIVIDE, POWER,
    SEMI_COLON,
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    ID, NUM,
    ENDFILE, ERROR
};

// Used for debugging only /////////////////////////////////////////////////////////
const char* TokenTypeStr[]=
        {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",
                "EndFile", "Error"
        };

struct Token
{
    TokenType type;
    char str[MAX_TOKEN_LEN+1];

    Token(){str[0]=0; type=ERROR;}
    Token(TokenType _type, const char* _str) {type=_type; Copy(str, _str);}
};

const Token reserved_words[]=
        {
                Token(IF, "if"),
                Token(THEN, "then"),
                Token(ELSE, "else"),
                Token(END, "end"),
                Token(REPEAT, "repeat"),
                Token(UNTIL, "until"),
                Token(READ, "read"),
                Token(WRITE, "write")
        };
const int num_reserved_words=sizeof(reserved_words)/sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[]=
        {
                Token(ASSIGN, ":="),
                Token(EQUAL, "="),
                Token(LESS_THAN, "<"),
                Token(PLUS, "+"),
                Token(MINUS, "-"),
                Token(TIMES, "*"),
                Token(DIVIDE, "/"),
                Token(POWER, "^"),
                Token(SEMI_COLON, ";"),
                Token(LEFT_PAREN, "("),
                Token(RIGHT_PAREN, ")"),
                Token(LEFT_BRACE, "{"),
                Token(RIGHT_BRACE, "}")
        };
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch){return (ch>='0' && ch<='9');}
inline bool IsLetter(char ch){return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));}
inline bool IsLetterOrUnderscore(char ch){return (IsLetter(ch) || ch=='_');}

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->str[0]=0;

    int i;
    char* s=pci->in_file.GetNextTokenStr();
    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->str[0]=0;
        return;
    }

    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if(i<num_symbolic_tokens)
    {
        if(symbolic_tokens[i].type==LEFT_BRACE)
        {
            pci->in_file.Advance(strlen(symbolic_tokens[i].str));
            if(!pci->in_file.SkipUpto(symbolic_tokens[i+1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type=symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    }
    else if(IsDigit(s[0]))
    {
        int j=1;
        while(IsDigit(s[j])) j++;

        ptoken->type=NUM;
        Copy(ptoken->str, s, j);
    }
    else if(IsLetterOrUnderscore(s[0]))
    {
        int j=1;
        while(IsLetterOrUnderscore(s[j])) j++;

        ptoken->type=ID;
        Copy(ptoken->str, s, j);

        for(i=0;i<num_reserved_words;i++)
        {
            if(Equals(ptoken->str, reserved_words[i].str))
            {
                ptoken->type=reserved_words[i].type;
                break;
            }
        }
    }

    int len=strlen(ptoken->str);
    if(len>0) pci->in_file.Advance(len);
}

// Parser //////////////////////////////////////////////////////////////////////////


enum NodeKind{
    IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
    OPER_NODE, NUM_NODE, ID_NODE
};

// Used for debugging only /////////////////////////////////////////////////////////
const char* NodeKindStr[]=
        {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
        };

enum ExprDataType {VOID, INTEGER, BOOLEAN};

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[]=
        {
                "Void", "Integer", "Boolean"
        };

#define MAX_CHILDREN 3//terminal

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];//terminal
    TreeNode* sibling; // used for sibling statements only

    NodeKind node_kind;

    union{TokenType oper; int num; char* id;}; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num;

    TreeNode() {int i; for(i=0;i<MAX_CHILDREN;i++) child[i]=0; sibling=0; expr_data_type=VOID;}
};

struct ParseInfo
{
    Token next_token;
};

void PrintTree(TreeNode* node, int sh=0)
{
    int i, NSH=3;
    for(i=0;i<sh;i++) printf(" ");

    printf("[%s]", NodeKindStr[node->node_kind]);

    if(node->node_kind==OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if(node->node_kind==NUM_NODE) printf("[%d]", node->num);
    else if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE) printf("[%s]", node->id);

    if(node->expr_data_type!=VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) PrintTree(node->child[i], sh+NSH);
    if(node->sibling) PrintTree(node->sibling, sh);
}

deque<pair<TokenType,string>>Tokens;
pair<TokenType,string> CToken;


pair<TokenType,string>  gettoken();
bool match(TokenType);

void program();
void stmtseq();
void stmt();
void ifstmt();
void exp();
void repeatstmt();
void readstmt();
void mathexpr();
void term();
void factor();
void newexpr();
void writestmt();
void assignstmt();


// program -> stmtseq
void program(){
    CToken = gettoken();
    stmtseq();
}

//stmtseq -> stmt { ; stmt }
void stmtseq(){
    stmt();
    while (CToken.first!=ENDFILE && CToken.first!=END && CToken.first!=UNTIL)
    {
        match(SEMI_COLON);
        stmt();
    }
}

// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
void stmt(){

    switch (CToken.first)
    {
        case IF:
            cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
            ifstmt();
            break;
        case REPEAT:
            cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
            repeatstmt();
            break;
        case ID:
            cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
            assignstmt();
            break;
        case READ:
            cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
            readstmt();
            break;
        case WRITE:
            cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
            writestmt();
            break;
        default:
            match(CToken.first);
            break;
    }
}

// ifstmt -> if expr then stmtseq [ else stmtseq ] end
void ifstmt(){

    match(IF);// c = NUM

    exp();

    cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
    match(THEN);

    stmtseq();

    if (CToken.first==ELSE){
        cout<<TokenTypeStr[CToken.first]<<endl;
        stmtseq();
    }

    match(END);
}

// repeatstmt -> repeat stmtseq until expr
void repeatstmt(){

    match(REPEAT);

    stmtseq();

    match(UNTIL);

    exp();
}

// assignstmt -> identifier := expr
void assignstmt(){
    match(ID);
    cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
    match(ASSIGN);
    exp();
}

// readstmt -> read identifier
void readstmt(){
    match(READ);
    cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
    match(ID);//c = SEMI_COLM
}

// writestmt -> write expr
void writestmt(){
    match(WRITE);
    exp();
}

// expr -> mathexpr [ (<|=) mathexpr ]
void exp(){
    mathexpr();
    if (CToken.first==LESS_THAN || CToken.first==EQUAL){
        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(CToken.first);
        mathexpr();
    }
}

// mathexpr -> term { (+|-) term }    left associative
void mathexpr(){
    term();

    if (CToken.first==PLUS)cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;

    if (CToken.first==MINUS)cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;


    while (match(PLUS) || match((MINUS))){
        match(CToken.first);
        term();
    }
}

// term -> factor { (*|/) factor }    left associative
void term(){
    factor();

    if (CToken.first==TIMES)cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
    if (CToken.first==DIVIDE)cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;


    while (match(TIMES) || match(DIVIDE)){
//        match(CToken.first);
        factor();
    }
}

// factor -> newexpr { ^ newexpr }    right associative
void factor(){
    newexpr();
    if (CToken.first==POWER)cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
    while (match(POWER)){
        newexpr();
    }
}

// newexpr -> ( mathexpr ) | number | identifier
void newexpr(){
    if (CToken.first==LEFT_PAREN){
        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(LEFT_PAREN);
        mathexpr();

        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(RIGHT_PAREN);
    }else if(CToken.first==NUM){
        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(NUM);
    }else if(CToken.first==ID){
        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(ID);
    }else {
        cout<<TokenTypeStr[CToken.first]<<" | "<<CToken.second<<endl;
        match(CToken.first);
    }
}


pair<TokenType,string> gettoken(){
    if (Tokens.empty())exit(0);
    auto t = Tokens.front();
    Tokens.pop_front();
    return t;
}

bool match(TokenType current){
    if (CToken.first==current){
        CToken = gettoken();
        return 1;
    }else
    {
//        cout<<"e"<<TokenTypeStr[current]<<endl;
        return 0;
    }
}

int main()
{
    freopen("input.txt", "r", stdin);

    auto *compiler = new CompilerInfo("input.txt","output.txt","debug.txt");
    auto *token = new Token();

    while (token->type!=ENDFILE)
    {
        GetNextToken(compiler,token);
        compiler->out_file.Out(token->str);

        Tokens.emplace_back(token->type,token->str);
    }
///////////////////////////
    freopen("output.txt", "r", stdin);
    freopen("Parser.txt", "w", stdout);

    program();

}