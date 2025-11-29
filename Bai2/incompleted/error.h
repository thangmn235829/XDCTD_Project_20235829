/* Error.h */
#ifndef __ERROR_H__
#define __ERROR_H__

// Danh sách các lỗi trong quá trình phân tích từ vựng
typedef enum {
  ERR_ENDOFCOMMENT,
  ERR_IDENTTOOLONG,
  ERR_INVALIDCHARCONSTANT,
  ERR_INVALIDSYMBOL,
  // Thêm mã lỗi mới
  ERR_STRINGTOOLONG,
  ERR_ENDOFQUOTEEXPECTED
} ErrorCode;

// Các thông báo lỗi
#define ERM_ENDOFCOMMENT "End of comment expected!"
#define ERM_IDENTTOOLONG "Identification too long!"
#define ERM_INVALIDCHARCONSTANT "Invalid const char!"
#define ERM_INVALIDSYMBOL "Invalid symbol!"
// Thêm thông báo lỗi mới
#define ERM_STRINGTOOLONG "Constant string too long (max 255 chars)!"
#define ERM_ENDOFQUOTEEXPECTED "Closing double quote expected!"

// Hàm thông báo lỗi
void error(ErrorCode err, int lineNo, int colNo);

#endif
