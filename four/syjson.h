#ifndef SYJSON_H__
#define SYJSON_H__
//数据类型
typedef enum
{
	SYJSON_NULL, SYJSON_TRUE, SYJSON_FALSE, SYJSON_NUM, SYJSON_STR, SYJSON_ARR, SYJSON_OBJ
} syjson_type;
//值结构
typedef struct
{
	syjson_type type;
	union
	{
		struct {char* s; size_t l;} str;
		double num;
	} val;
} syjson_value;
//错误类型
enum {
	SYJSON_PARSE_OK = 0,
	SYJSON_PARSE_EXPECT_VALUE,
	SYJSON_PARSE_INVALID_VALUE,
	SYJSON_PARSE_ROOT_NOT_SINGULAR,
	SYJSON_PARSE_NUMBER_TOO_BIG,
	SYJSON_PARSE_MISS_QUOTATION_MARK,
	SYJSON_PARSE_INVALID_STRING_ESCAPE,
	SYJSON_PARSE_INVALID_STRING_CHAR
};
//动态堆栈
typedef struct
{
	const char* json;
	char* stack;
	size_t size, top;
} syjson_content;

//初始化
#define syjson_init(v) do { (v)->type = SYJSON_NULL; } while(0)
//解析函数
int syjson_parse(syjson_value* v, const char* json);
//获取json值类型
syjson_type syjson_get_type(const syjson_value* v);
//获取布尔值
int syjson_get_boolean(const syjson_value* v);
//获取数字类型，防止不够用，使用double类型
double syjson_get_number(const syjson_value* v);
//获取字符串
const char* syjson_get_string(const syjson_value* v);
//获取字符串长度
size_t syjson_get_string_length(const syjson_value* v);
//置空
#define syjson_set_null(v) syjson_free(v);
//设置布尔值
void syjson_set_boolean(syjson_value* v, int b);
//设置数字
void syjson_set_number(syjson_value* v, double n);
//设置字符串
void syjson_set_string(syjson_value* v, const char* s, size_t l);

#endif
