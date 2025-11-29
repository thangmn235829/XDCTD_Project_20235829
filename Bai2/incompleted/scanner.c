/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> // Cần cho strncat/strlen và strtol

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/
// Cần giả định các hàm readIdentKeyword, readNumber, readConstChar cơ bản
// để đảm bảo file scanner.c là hoàn chỉnh.

void skipLine()
{
  while (currentChar != EOF && currentChar != '\n')
  {
    readChar();
  }
  if (currentChar != EOF)
    readChar();
}

void skipBlank()
{
  while (currentChar != EOF && charCodes[currentChar] == CHAR_SPACE)
    readChar();
}

void skipComment()
{
  int nestingLevel = 1;

  while (currentChar != EOF)
  {
    readChar();
    if (currentChar == EOF)
    {
      error(ERR_ENDOFCOMMENT, lineNo, colNo);
      return;
    }
    else if (charCodes[currentChar] == CHAR_TIMES)
    {
      readChar();
      if (currentChar == EOF)
      {
        error(ERR_ENDOFCOMMENT, lineNo, colNo);
        return;
      }
      else if (charCodes[currentChar] == CHAR_RPAR)
      {
        nestingLevel--;
        if (nestingLevel == 0)
        {
          readChar();
          return;
        }
      } 
    }
    else if (charCodes[currentChar] == CHAR_LPAR)
    {
      readChar();
      if (currentChar == EOF)
      {
        error(ERR_ENDOFCOMMENT, lineNo, colNo);
        return;
      }
      else if (charCodes[currentChar] == CHAR_TIMES)
      {
        nestingLevel++; 
      }
    }
  }
}

// -----------------------------------------------------
// CÁC HÀM PHỤ TRỢ CƠ BẢN (Triển khai tối thiểu để đảm bảo tính hoàn chỉnh)
// -----------------------------------------------------

Token* readIdentKeyword(void) {
  Token *token = makeToken(TK_IDENT, lineNo, colNo);
  int len = 0;

  while (currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT)) {
    if (len >= MAX_IDENT_LEN) {
      error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
    }
    token->string[len++] = currentChar;
    readChar();
  }
  token->string[len] = '\0';
  token->tokenType = checkKeyword(token->string);
  return token;
}

Token* readNumber(void) {
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  int len = 0;
  char numStr[20]; // Đủ lớn cho số nguyên

  while (currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT) {
    if (len >= 19) { // Kiểm tra tràn số cơ bản
      // Không cần phải là lỗi, nhưng để đơn giản ta giới hạn
    }
    numStr[len++] = currentChar;
    readChar();
  }
  numStr[len] = '\0';
  token->value = (int)strtol(numStr, NULL, 10);
  return token;
}

Token* readConstChar(void) {
  Token *token = makeToken(TK_CHAR, lineNo, colNo);
  readChar(); // Bỏ qua nháy đơn mở

  if (currentChar == EOF) {
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
  }
  
  if (currentChar == '\n') {
      error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
  }

  token->string[0] = currentChar;
  token->string[1] = '\0';
  readChar(); // Đọc ký tự của hằng

  if (currentChar == EOF || charCodes[currentChar] != CHAR_SINGLEQUOTE) {
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
  }

  readChar(); // Bỏ qua nháy đơn đóng
  return token;
}

// -----------------------------------------------------
// HÀM MỚI: readStringConstant (Xử lý hằng xâu)
// -----------------------------------------------------

Token* readStringConstant(void) {
  Token *token = makeToken(TK_STRING, lineNo, colNo);
  int len = 0;
  
  readChar(); // Bỏ qua dấu nháy kép mở đầu (")
  
  while (currentChar != EOF) {
    
    // 1. Gặp dấu nháy kép đóng
    if (charCodes[currentChar] == CHAR_DOUBLEQUOTE) {
      token->string[len] = '\0';
      readChar(); // Bỏ qua dấu nháy kép đóng
      return token;
    }
    
    // 2. Gặp ký tự xuống dòng ('\n')
    if (currentChar == '\n') {
      // 2a. Tăng dòng và cột (vì vẫn là phần của mã nguồn)
      lineNo++;
      colNo = 0;
      readChar(); // Đọc ký tự tiếp theo sau \n
      
      // 2b. Bỏ qua các ký tự trắng (space/tab) cho đến khi gặp ký tự khác trắng
      // Lưu ý: Chỉ bỏ qua space/tab, không bỏ qua \n vì \n vừa được xử lý.
      while (currentChar != EOF && currentChar != '"' && (currentChar == ' ' || currentChar == '\t')) {
        readChar();
      }
      
      // Ký tự hiện tại không phải là \n, space, tab và cũng không phải là EOF/kết thúc chuỗi,
      // nó sẽ được xử lý ở vòng lặp tiếp theo.
      continue;
    }
    
    // 3. Kiểm tra độ dài xâu
    if (len >= MAX_STRING_LENGTH) {
      error(ERR_STRINGTOOLONG, token->lineNo, token->colNo);
      // Dù báo lỗi, vẫn tiếp tục đọc cho đến khi đóng quote để tránh lỗi tiếp theo
    }
    
    // 4. Thêm ký tự vào xâu
    if (len < MAX_STRING_LENGTH) {
        token->string[len] = currentChar;
        len++;
    }
    readChar();
  }
  
  // Gặp EOF mà chưa có dấu nháy kép đóng
  token->string[len] = '\0';
  error(ERR_ENDOFQUOTEEXPECTED, token->lineNo, token->colNo);
  return token; 
}


// -----------------------------------------------------
// HÀM CHÍNH: getToken()
// -----------------------------------------------------

Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF)
    return makeToken(TK_EOF, lineNo, colNo);

  ln = lineNo;
  cn = colNo;

  switch (charCodes[currentChar]) {
    case CHAR_SPACE: 
      skipBlank(); 
      return getToken();
    case CHAR_LETTER: 
      return readIdentKeyword();
    case CHAR_DIGIT: 
      return readNumber();
      
    // Thêm trường hợp cho hằng xâu ký tự
    case CHAR_DOUBLEQUOTE:
      return readStringConstant();

    case CHAR_PLUS: 
      token = makeToken(SB_PLUS, ln, cn);
      readChar(); 
      return token;
    case CHAR_MINUS: 
      token = makeToken(SB_MINUS, ln, cn);
      readChar(); 
      return token;

    case CHAR_TIMES:
      readChar(); 
      if (charCodes[currentChar] == CHAR_TIMES) {
          // Gặp '**', toán tử lũy thừa
          token = makeToken(SB_EXPONENT, ln, cn);
          readChar();
          return token;
      } else {
          // Chỉ là toán tử nhân đơn '*'
          return makeToken(SB_TIMES, ln, cn);
      }

    case CHAR_SLASH:
      readChar(); 
      if (charCodes[currentChar] == CHAR_SLASH) {
          // Gặp '//', chú thích một dòng
          skipLine(); 
          return getToken();
      } else {
          // Chỉ là toán tử chia đơn '/'
          return makeToken(SB_SLASH, ln, cn);
      }
      
    case CHAR_PERCENTAGE:
      // Gặp '%', toán tử chia lấy dư
      token = makeToken(SB_MOD, ln, cn);
      readChar();
      return token;

    case CHAR_LT:
      readChar();
      if (currentChar == '=') {
        token = makeToken(SB_LE, ln, cn);
        readChar();
        return token;
      } else {
        return makeToken(SB_LT, ln, cn);
      }

    case CHAR_GT:
      readChar();
      if (currentChar == '=') {
        token = makeToken(SB_GE, ln, cn);
        readChar();
        return token;
      } else {
        return makeToken(SB_GT, ln, cn);
      }

    case CHAR_EXCLAIMATION:
      readChar();
      if (currentChar == '=') {
        token = makeToken(SB_NEQ, ln, cn);
        readChar();
        return token;
      } else {
        token = makeToken(TK_NONE, ln, cn);
        error(ERR_INVALIDSYMBOL, ln, cn);
        return token;
      }

    case CHAR_EQ:
      token = makeToken(SB_EQ, ln, cn);
      readChar();
      return token;

    case CHAR_COMMA:
      token = makeToken(SB_COMMA, ln, cn);
      readChar();
      return token;

    case CHAR_PERIOD:
      readChar();
      if (charCodes[currentChar] == CHAR_RPAR) {
        token = makeToken(SB_RSEL, ln, cn);
        readChar();
        return token;
      } else {
        return makeToken(SB_PERIOD, ln, cn);
      }

    case CHAR_COLON:
      readChar();
      if (currentChar == '=') {
        token = makeToken(SB_ASSIGN, ln, cn);
        readChar();
        return token;
      } else {
        return makeToken(SB_COLON, ln, cn);
      }

    case CHAR_SEMICOLON:
      token = makeToken(SB_SEMICOLON, ln, cn);
      readChar();
      return token;
      
    case CHAR_SINGLEQUOTE:
      return readConstChar(); 

    case CHAR_LPAR:
      readChar();
      switch (charCodes[currentChar]) {
        case CHAR_PERIOD:
          token = makeToken(SB_LSEL, ln, cn);
          readChar();
          return token;
        case CHAR_TIMES:
          skipComment(); // Chú thích đa dòng (*...*)
          return getToken();
        default:
          return makeToken(SB_LPAR, ln, cn);
      }

    case CHAR_RPAR:
      token = makeToken(SB_RPAR, ln, cn);
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

void printToken(Token *token)
{

  // KhÃ´ng in token lá»—i (TK_NONE)
  if (token->tokenType == TK_NONE)
    return;

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType)
  {
  case TK_NONE:
    printf("TK_NONE\n");
    break;
  case TK_IDENT:
    printf("TK_IDENT(%s)\n", token->string);
    break;
  case TK_NUMBER:
    printf("TK_NUMBER(%d)\n", token->value);
    break;
  case TK_CHAR:
    printf("TK_CHAR('%s')\n", token->string);
    break;
  case TK_EOF:
    printf("TK_EOF\n");
    break;

  case KW_PROGRAM:
    printf("KW_PROGRAM\n");
    break;
  case KW_CONST:
    printf("KW_CONST\n");
    break;
  case KW_TYPE:
    printf("KW_TYPE\n");
    break;
  case KW_VAR:
    printf("KW_VAR\n");
    break;
  case KW_INTEGER:
    printf("KW_INTEGER\n");
    break;
  case KW_CHAR:
    printf("KW_CHAR\n");
    break;
  case KW_ARRAY:
    printf("KW_ARRAY\n");
    break;
  case KW_OF:
    printf("KW_OF\n");
    break;
  case KW_FUNCTION:
    printf("KW_FUNCTION\n");
    break;
  case KW_PROCEDURE:
    printf("KW_PROCEDURE\n");
    break;
  case KW_BEGIN:
    printf("KW_BEGIN\n");
    break;
  case KW_END:
    printf("KW_END\n");
    break;
  case KW_CALL:
    printf("KW_CALL\n");
    break;
  case KW_IF:
    printf("KW_IF\n");
    break;
  case KW_THEN:
    printf("KW_THEN\n");
    break;
  case KW_ELSE:
    printf("KW_ELSE\n");
    break;
  case KW_WHILE:
    printf("KW_WHILE\n");
    break;
  case KW_DO:
    printf("KW_DO\n");
    break;
  case KW_FOR:
    printf("KW_FOR\n");
    break;
  case KW_TO:
    printf("KW_TO\n");
    break;

  case SB_SEMICOLON:
    printf("SB_SEMICOLON\n");
    break;
  case SB_COLON:
    printf("SB_COLON\n");
    break;
  case SB_PERIOD:
    printf("SB_PERIOD\n");
    break;
  case SB_COMMA:
    printf("SB_COMMA\n");
    break;
  case SB_ASSIGN:
    printf("SB_ASSIGN\n");
    break;
  case SB_EQ:
    printf("SB_EQ\n");
    break;
  case SB_NEQ:
    printf("SB_NEQ\n");
    break;
  case SB_LT:
    printf("SB_LT\n");
    break;
  case SB_LE:
    printf("SB_LE\n");
    break;
  case SB_GT:
    printf("SB_GT\n");
    break;
  case SB_GE:
    printf("SB_GE\n");
    break;
  case SB_PLUS:
    printf("SB_PLUS\n");
    break;
  case SB_MINUS:
    printf("SB_MINUS\n");
    break;
  case SB_TIMES:
    printf("SB_TIMES\n");
    break;
  case SB_SLASH:
    printf("SB_SLASH\n");
    break;
  case SB_LPAR:
    printf("SB_LPAR\n");
    break;
  case SB_RPAR:
    printf("SB_RPAR\n");
    break;
  case SB_LSEL:
    printf("SB_LSEL\n");
    break;
  case SB_RSEL:
    printf("SB_RSEL\n");
    break;
  }
}

int scan(char *fileName)
{
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF)
  {
    printToken(token);
    free(token);
    token = getToken();
  }

  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[])
{
  if (argc <= 1)
  {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR)
  {
    printf("Can\'t read input file!\n");
    return -1;
  }

  return 0;
}
