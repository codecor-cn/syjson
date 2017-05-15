#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syjson.h"

static int main_result = 0;
static int test_count = 0;
static int test_pass = 0;

//基础检验计数宏
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
//类型检验宏
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_TRUE(expect) EXPECT_EQ_BASE((expect) != 0, "true", "false", "%s")
#define EXPECT_EQ_FALSE(expect) EXPECT_EQ_BASE((expect) == 0, "false", "true", "%s")
#define EXPECT_EQ_STRING(expect, autual, alen) EXPECT_EQ_BASE(sizeof(expect) -1 == alen && memcmp(expect, autual, alen) == 0, expect, autual, "%s");
#define EXPECT_EQ_ARRAY(expect, autual, alen) EXPECT_EQ_BASE(sizeof(expect) -1 == alen && memcmp(expect, autual, alen) == 0, expect, autual, "%s");

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
	//检测UNICODE解析
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");//美元 U+0024
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");//美分 U+00A2
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");//欧元 U+20AC
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");//辅助平面UTF-8，U+1D11E，有的终端支持不全
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");//U+1D11E，同上
}

#define EXPECT_EQ_ARRAY_SIZE(SIZE, ARRAY_SIZE) EXPECT_EQ_BASE()
//测试解析数组
static void test_parse_array()
{
	syjson_value v;

	syjson_init(&v);
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "[ ]"));
	EXPECT_EQ_INT(SYJSON_ARRAY, syjson_get_type(&v));
	EXPECT_EQ_ARRAY_SIZE(0, syjson_get_array_size(&v));
	syjson_free(&v);
}

//测试语法错误
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

	//错误数组
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "[1,");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "[\"a\", nul]");

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
//测试字符串无效UNICODE
static void test_parse_invalid_unicode_hex()
{
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}
//测试字符串无效代理码点
static void test_parse_invalid_unicode_surrogate()
{
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}
//测试数组丢失后错误
static void test_parse_miss_comma_or_square_bracket()
{
	TEST_ERROR(SYJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(SYJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(SYJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(SYJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
	TEST_ERROR(SYJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[,");
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
//测试获取数组
static void test_access_array()
{
	//TODO 检测数组获取情况
	/*
	syjson_value v;
	syjson_init(&v);
	syjson_set_array(&v, "[]", 1);
	EXPECT_EQ_ARRAY("[]", syjson_get_string(&v), 1);
	*/
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
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
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
