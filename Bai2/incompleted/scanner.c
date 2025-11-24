/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank() {
  while (charCodes[currentChar] == CHAR_SPACE)
    readChar();
}

void skipComment() {
  /* Assumes currentChar is the '*' after '(' that started the comment */
  readChar(); /* consume the '*' */

  while (currentChar != EOF) {
    if (charCodes[currentChar] == CHAR_TIMES) {
      readChar(); /* consume '*' */
      if (currentChar != EOF && charCodes[currentChar] == CHAR_RPAR) {
        readChar(); /* consume ')' and exit comment */
        return;
      }
    } else {
      readChar();
    }
  }

  /* EOF reached before closing "*)" */
  error(ERR_ENDOFCOMMENT, lineNo, colNo);
}

Token* readIdentKeyword(void) {
  // TODO
  Token *token = makeToken(TK_IDENT, lineNo, colNo);
  int i = 0;
  int tooLong = 0;

  while (charCodes[currentChar] == CHAR_LETTER ||
         charCodes[currentChar] == CHAR_DIGIT) {
    if (i < MAX_IDENT_LEN) {
      token->string[i++] = (char) currentChar;
    } else {
      tooLong = 1; /* still consume remaining identifier chars but don't store */
    }
    readChar();
  }

  token->string[i] = '\0';

  if (tooLong) {
    token->tokenType = TK_NONE;
    error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
    return token;
  }

  /* check if the identifier is a keyword */
  TokenType kt = checkKeyword(token->string);
  if (kt != TK_NONE) token->tokenType = kt;

  return token;
}

Token* readNumber(void) {
  // TODO
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  int i = 0;

  while (charCodes[currentChar] == CHAR_DIGIT) {
    if (i < MAX_IDENT_LEN) {
      token->string[i++] = (char)currentChar;
    } else {
      /* too many digits for token->string: consume remaining digits,
         report error and return TK_NONE (reuse ERR_IDENTTOOLONG) */
      readChar();
      while (charCodes[currentChar] == CHAR_DIGIT) readChar();
      token->string[MAX_IDENT_LEN] = '\0';
      token->tokenType = TK_NONE;
      error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
      return token;
    }
    readChar();
  }

  token->string[i] = '\0';
  token->value = atoi(token->string);
  return token;
}

Token* readConstChar(void) {
  // TODO
   Token *token = makeToken(TK_CHAR, lineNo, colNo);

  readChar();
  if (currentChar == EOF) {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }
    
  token->string[0] = currentChar;
  token->string[1] = '\0';

  readChar();
  if (currentChar == EOF) {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }

  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    readChar();
    return token;
  } else {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
    return token;
  }
}


// Giả định các hàm sau đã được định nghĩa trong các tệp khác:
// Token* makeToken(TokenType tokenType, int lineNo, int colNo);
// void readChar(void);
// void error(ErrorCode err, int lineNo, int colNo);

Token* getToken(void) {
  Token *token;
  int ln = lineNo; // Lưu vị trí bắt đầu token
  int cn = colNo;

  if (currentChar == EOF)
    return makeToken(TK_EOF, ln, cn);

  switch (charCodes[currentChar]) {
    case CHAR_SPACE: 
      skipBlank(); 
      return getToken();
    case CHAR_LETTER: 
      return readIdentKeyword();
    case CHAR_DIGIT: 
      return readNumber();
    case CHAR_PLUS: 
      token = makeToken(SB_PLUS, ln, cn);
      readChar(); 
      return token;
    case CHAR_MINUS: 
      token = makeToken(SB_MINUS, ln, cn);
      readChar(); 
      return token;

    case CHAR_TIMES:
      // Token đơn: Toán tử nhân '*' (SB_TIMES)
      token = makeToken(SB_TIMES, ln, cn);
      readChar();
      return token;

    case CHAR_SLASH:
      // Token đơn: Toán tử chia '/' (SB_SLASH)
      token = makeToken(SB_SLASH, ln, cn);
      readChar();
      return token;

    case CHAR_LT:
      readChar(); // Đọc '<'
      if (currentChar == '=') {
        // Token hai ký tự: Toán tử <= (SB_LE)
        token = makeToken(SB_LE, ln, cn);
        readChar();
        return token;
      } else {
        // Token đơn: Toán tử < (SB_LT)
        return makeToken(SB_LT, ln, cn);
      }

    case CHAR_GT:
      readChar(); // Đọc '>'
      if (currentChar == '=') {
        // Token hai ký tự: Toán tử >= (SB_GE)
        token = makeToken(SB_GE, ln, cn);
        readChar();
        return token;
      } else {
        // Token đơn: Toán tử > (SB_GT)
        return makeToken(SB_GT, ln, cn);
      }

    case CHAR_EXCLAIMATION:
      readChar(); // Đọc '!'
      if (currentChar == '=') {
        // Token hai ký tự: Toán tử != (SB_NEQ)
        token = makeToken(SB_NEQ, ln, cn);
        readChar();
        return token;
      } else {
        // Ký tự '!' không hợp lệ nếu không theo sau bởi '='
        token = makeToken(TK_NONE, ln, cn);
        error(ERR_INVALIDSYMBOL, ln, cn);
        return token;
      }

    case CHAR_EQ:
      // Token đơn: Toán tử = (SB_EQ)
      token = makeToken(SB_EQ, ln, cn);
      readChar();
      return token;

    case CHAR_COMMA:
      // Token đơn: Dấu phẩy ',' (SB_COMMA)
      token = makeToken(SB_COMMA, ln, cn);
      readChar();
      return token;

    case CHAR_PERIOD:
      readChar(); // Đọc '.'
      if (currentChar == ')') {
        // Token hai ký tự: Chỉ mục mảng đóng '.)' (SB_RSEL)
        token = makeToken(SB_RSEL, ln, cn);
        readChar();
        return token;
      } else {
        // Token đơn: Dấu chấm '.' (SB_PERIOD)
        return makeToken(SB_PERIOD, ln, cn);
      }

    case CHAR_COLON:
      readChar(); // Đọc ':'
      if (currentChar == '=') {
        // Token hai ký tự: Toán tử gán ':=' (SB_ASSIGN)
        token = makeToken(SB_ASSIGN, ln, cn);
        readChar();
        return token;
      } else {
        // Token đơn: Dấu hai chấm ':' (SB_COLON)
        return makeToken(SB_COLON, ln, cn);
      }

    case CHAR_SEMICOLON:
      // Token đơn: Dấu chấm phẩy ';' (SB_SEMICOLON)
      token = makeToken(SB_SEMICOLON, ln, cn);
      readChar();
      return token;
      
    case CHAR_SINGLEQUOTE:
      // Hằng ký tự đơn 'c'
      return readConstChar(); // Hàm này phải xử lý việc đọc ký tự và dấu đóng '

    case CHAR_LPAR:
      readChar(); // Đọc '('
      switch (charCodes[currentChar]) {
        case CHAR_PERIOD:
          // Token hai ký tự: Chỉ mục mảng mở '(. ' (SB_LSEL)
          token = makeToken(SB_LSEL, ln, cn);
          readChar();
          return token;
        case CHAR_TIMES:
          // Bắt đầu chú thích '(*' (Bỏ qua và gọi lại getToken)
          skipComment(); 
          return getToken();
        default:
          // Token đơn: Dấu ngoặc đơn mở '(' (SB_LPAR)
          return makeToken(SB_LPAR, ln, cn);
      }

    case CHAR_RPAR:
      // Token đơn: Dấu ngoặc đơn đóng ')' (SB_RPAR)
      token = makeToken(SB_RPAR, ln, cn);
      readChar();
      return token;
      
    // KẾT THÚC PHẦN HOÀN THIỆN

    case CHAR_UNKNOWN:
      token = makeToken(TK_NONE, ln, cn);
      error(ERR_INVALIDSYMBOL, ln, cn);
      readChar();
      return token;
      
    default:
      token = makeToken(TK_NONE, ln, cn);
      error(ERR_INVALIDSYMBOL, ln, cn);
      readChar();
      return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%d)\n", token->value); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}



