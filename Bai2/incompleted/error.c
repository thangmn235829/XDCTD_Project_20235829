/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void error(ErrorCode err, int lineNo, int colNo)
{
  switch (err)
  {
  case ERR_ENDOFCOMMENT:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_ENDOFCOMMENT);
    break;
  case ERR_IDENTTOOLONG:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_IDENTTOOLONG);
    break;
  case ERR_INVALIDCHARCONSTANT:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALIDCHARCONSTANT);
    break;
  case ERR_INVALIDSYMBOL:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALIDSYMBOL);
    break;
  // Xử lý các lỗi mới
  case ERR_STRINGTOOLONG:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_STRINGTOOLONG);
    break;
  case ERR_ENDOFQUOTEEXPECTED:
    printf("%d-%d:%s\n", lineNo, colNo, ERM_ENDOFQUOTEEXPECTED);
    break;
  }
  // Thoát chương trình ngay khi có lỗi
  exit(-1);
}
