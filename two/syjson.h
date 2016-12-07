#ifndef SYJSON_01_H__
#define SYJSON_01_H__

typedef enum { SYJSON_NULL, SYJSON_TRUE, SYJSON_FALSE, SYJSON_NUM, SYJSON_STR, SYJSON_ARR, SYJSON_OBJ } syjson_type;

typedef struct {
	syjson_type type;
	double num;
}syjson_value;

enum {
	SYJSON_PARSE_OK = 0,
	SYJSON_PARSE_EXPECT_VALUE,
	SYJSON_PARSE_INVALID_VALUE,
	SYJSON_PARSE_ROOT_NOT_SINGULAR,
	SYJSON_PARSE_NUMBER_TOO_BIG
};

//解析函数
int syjson_parse(syjson_value* v, const char* json);
//获取json值类型
syjson_type syjson_get_type(const syjson_value* v);
//获取数字类型，防止不够用，使用double类型
double syjson_get_number(const syjson_value* v);

#endif
