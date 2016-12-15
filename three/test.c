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
				fprintf(stderr, "%s:%d: 想要的类型:"format" 实际的类型:"format"\n", __FILE__, __LINE__, expect, actual);\
				main_result++;\
			}\
		} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%s")
//测试NULL类型数据
static void test_parse_null()
{
	syjson_value v;
	v.type = SYJSON_FALSE;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "null"));
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
}
//测试FALSE类型数据
static void test_parse_false()
{
	syjson_value v;
	v.type = SYJSON_NULL;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "false"));
	EXPECT_EQ_INT(SYJSON_FALSE, syjson_get_type(&v));
}
//测试TRUE类型数据
static void test_parse_true()
{
	syjson_value v;
	v.type = SYJSON_NULL;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "true"));
	EXPECT_EQ_INT(SYJSON_TRUE, syjson_get_type(&v));
}
//测试数字类型
#define TEST_NUMBER(expect, json)\
	do{\
		syjson_value v;\
		EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, json));\
		EXPECT_EQ_INT(SYJSON_NUM, syjson_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, syjson_get_number(&v));\
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
}
#define EXPECT_EQ_STRING(s, str, l)\
		do{\
			EXPECT_EQ_INT(s, str);\
			EXPECT_EQ_INT(sizeof s, l);\
		}while(0)
//测试字符串
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
#define TEST_ERROR(error, json)\
		do{\
			syjson_value v;\
			v.type = SYJSON_FALSE;\
			EXPECT_EQ_INT(error, syjson_parse(&v, json));\
			EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));\
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

#if 1
	//invalid number
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(SYJSON_PARSE_INVALID_VALUE, "nan");
#endif
}
//json格式错误
static void test_parse_root_not_singular()
{
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "null a");

#if 1
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0123");//0之后只能是点或者为空
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(SYJSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}
//测试数字类型溢出
static void test_parse_number_too_big()
{
#if 1
	TEST_ERROR(SYJSON_PARSE_NUMBER_TOO_BIG, "1e999");
	TEST_ERROR(SYJSON_PARSE_NUMBER_TOO_BIG, "-1e999");
#endif
}
//测试函数入口
static void test_parse()
{
	//数据错误
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	//函数错误
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
}
//主函数
int main(int argc, char** argv)
{
	test_parse();
	printf("%d/%d (%3.2f%%) 通过\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return 1;
}
