#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syjson.h"

static int main_result = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equlity, expect, actual, format)\
		do{\
			test_count++;\
			if(equlity)\
				test_pass++;\
			else\
			{\
				fprintf(stderr, "%s:%d: 想要的类型值:"format" 实际的类型值:"format"\n", __FILE__, __LINE__, expect, actual);\
				main_result = 1;\
			}\
		} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_TRUE(expect) EXPECT_EQ_BASE((expect) != 0, "true", "false", "%s")
#define EXPECT_EQ_FALSE(expect) EXPECT_EQ_BASE((expect) == 0, "false", "true", "%s")
#define EXPECT_EQ_STRING(expect, autual, alen) EXPECT_EQ_BASE(sizeof(expect) -1 == alen && memcmp(expect, autual, alen) == 0, expect, autual, "%s");

//测试NULL类型数据
static void test_parse_null()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_boolean(&v, SYJSON_FALSE);
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "null"));
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
	syjson_free(&v);
}
//测试FALSE类型数据
static void test_parse_false()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_boolean(&v, SYJSON_TRUE);
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "false"));
	EXPECT_EQ_INT(SYJSON_FALSE, syjson_get_type(&v));
	syjson_free(&v);
}
//测试TRUE类型数据
static void test_parse_true()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_boolean(&v, SYJSON_FALSE);
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "true"));
	EXPECT_EQ_INT(SYJSON_TRUE, syjson_get_type(&v));
	syjson_free(&v);
}
//测试数字类型
#define TEST_NUMBER(expect, json)\
	do{\
		syjson_value v;\
		syjson_init(&v);\
		EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, json));\
		EXPECT_EQ_INT(SYJSON_NUM, syjson_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, syjson_get_number(&v));\
		syjson_free(&v);\
	}while(0)
//测试解析数字
static void test_parse_number()
{
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000");//最小数溢出
	//双精度浮点数越界检查
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json) \
			do {\
				syjson_value v;\
				syjson_init(&v);\
				EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, json));\
				EXPECT_EQ_INT(SYJSON_STR, syjson_get_type(&v));\
				EXPECT_EQ_STRING(expect, syjson_get_string(&v), syjson_get_string_length(&v));\
				syjson_free(&v);\
			} while(0)
//测试字符串
static void test_parse_string()
{
	TEST_STRING("", "\"\"");
	TEST_STRING("hello", "\"hello\"");
	TEST_STRING("hello\nworld", "\"hello\\nworld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}
#define TEST_ERROR(error_code, json)\
		do {\
			syjson_value v;\
			syjson_init(&v);\
			syjson_set_boolean(&v, SYJSON_FALSE);\
			EXPECT_EQ_INT(error_code, syjson_parse(&v, json));\
			EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));\
			syjson_free(&v);\
		}while(0)
//测试错误空数据
static void test_parse_expect_value()
{
	TEST_ERROR(SYJSON_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(SYJSON_PARSE_EXPECT_VALUE, " ");
}
//不能解析的数据
static void test_parse_invalid_value()
{
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "??");

	//无效的数字
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "nan");
}
//json格式错误
static void test_parse_root_not_singular()
{
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "null a");
	//无效的数字
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0123");//0之后只能是点或者为空
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
}
//测试数字类型溢出
static void test_parse_number_too_big()
{
	TEST_ERROR(SYJSON_PARSE_NUMBER_TOO_BIG, "1e999");
	TEST_ERROR(SYJSON_PARSE_NUMBER_TOO_BIG, "-1e999");
}
//测试字符串双引号
static void test_parse_missing_quotation_mark()
{
	TEST_ERROR(SYJSON_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(SYJSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}
//测试字符串无效转义
static void test_parse_invalid_string_escape()
{
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}
//测试字符串无效字符
static void test_parse_invalid_string_char()
{
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

//测试获取NULL类型
static void test_access_null()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_string(&v, "a", 1);
	syjson_set_null(&v);
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
	syjson_free(&v);
}
//测试获取布尔值
static void test_access_boolean()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_string(&v, "a", 1);
	syjson_set_boolean(&v, SYJSON_TRUE);
	EXPECT_EQ_TRUE(syjson_get_boolean(&v));
	syjson_set_boolean(&v, SYJSON_FALSE);
	EXPECT_EQ_FALSE(syjson_get_boolean(&v));
	syjson_free(&v);
}
//测试获取数字
static void test_access_number()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_string(&v, "a", 1);
	syjson_set_number(&v, 1234.5);
	EXPECT_EQ_DOUBLE(1234.5, syjson_get_number(&v));
	syjson_free(&v);
}
//测试获取字符串
static void test_access_string()
{
	syjson_value v;
	syjson_init(&v);
	syjson_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", syjson_get_string(&v), syjson_get_string_length(&v));
	syjson_set_string(&v, "hello", 5);
	EXPECT_EQ_STRING("hello", syjson_get_string(&v), syjson_get_string_length(&v));
	syjson_free(&v);
}
//测试函数入口
static void test_parse()
{
	//解析json测试
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	//错误测试
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	//测试获取值
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}
//主函数
int main(int argc, char** argv)
{
	test_parse();
	printf("%d/%d (%3.2f%%) 通过\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_result;
}
