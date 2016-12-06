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
//测试NULL类型数据
static void test_parse_null()
{
	syjson_value v;
	v.type = SYJSON_FALSE;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "null"));
	EXPRCT_EQ_INT(SYJSON_NULL, syjson_get_type($v));
}
//测试FALSE类型数据
static void test_parse_false()
{
	syjson_value v;
	v.type = SYJSON_NULL;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "false"));
	EXPRCT_EQ_INT(SYJSON_FALSE, syjson_get_type($v));
}
//测试TRUE类型数据
static void test_parse_true()
{
	syjson_value v;
	v.type = SYJSON_NULL;
	EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, "true"));
	EXPRCT_EQ_INT(SYJSON_TRUE, syjson_get_type($v));
}
//测试数字类型
#define TEST_NUMBER(expect, json)\
	do{\
		syjson_value v;\
		EXPECT_EQ_INT(SYJSON_PARSE_OK, syjson_parse(&v, json));\
		EXPECT_EQ_INT(SYJSON_NUM, syjson_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, syjson_get_num(&v));\
	}while(0)

//测试错误空数据
static void test_parse_expect_value()
{
	syjson_value v;
	v.type = SYJSON_TRUE;
	EXPECT_EQ_INT(SYJSON_PARSE_EXPECT_VALUE, syjson_parse(&v, ""));
	EXPRCT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));

	v.type = SYJSON_TRUE;
	EXPECT_EQ_INT(SYJSON_PARSE_EXPECT_VALUE, syjson_parse(&v, " "));
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
}
//不能解析的数据
static void test_parse_invalid_value()
{
	syjson_value v;
	v.type = SYJSON_TRUE;
	EXPECT_EQ_INT(SYJSON_PARSE_INVALID_VALUE, syjson_parse(&v, "nu"));
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));

	v.type = SYJSON_FALSE;
	EXPECT_EQ_INT(SYJSON_PARSE_INVALID_VALUE, syjson_parse(&v, "hehe"));
	EXPRCT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
}
//json格式错误
static void test_parse_root_not_singular()
{
	syjson_value v;
	v.type = SYJSON_TRUE;
	EXPECT_EQ_INT(SYJSON_PARSE_ROOT_NOT_SINGULAR, syjson_parse(&v, "null a"));
	EXPECT_EQ_INT(SYJSON_NULL, syjson_get_type(&v));
}
//解析函数
static void test_parse()
{
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
}
//主函数
int main(int argc, char** argv)
{
	test_parse()
	printf("%d/%d (%3.2f%%) 通过\n", test_pass, test_count, test_pass * 100 / test_count);
	return 1;
}
