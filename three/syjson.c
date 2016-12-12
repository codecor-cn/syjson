#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include "syjson.h"

#define EXPECT(c, ch)  do{ assert(*c->json == (ch)); c->json++; }while(0)

typedef struct
{
	const char* json;
}syjson_content;

//过滤空白
static void syjson_parse_whitespace(syjson_content* c)
{
	const char* p = c->json;
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}
//解析null类型
static int syjson_parse_null(syjson_content* c, syjson_value* v)
{
	EXPECT(c, 'n');
	if(c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
		return SYJSON_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = SYJSON_NULL;
	return SYJSON_PARSE_OK;
}
//解析FALSE类型
static int syjson_parse_false(syjson_content* c, syjson_value* v)
{
	EXPECT(c, 'f');
	if(c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
		return SYJSON_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = SYJSON_FALSE;
	return SYJSON_PARSE_OK;
}
//解析TRUE类型
static int syjson_parse_true(syjson_content* c, syjson_value* v)
{
	EXPECT(c, 't');
	if(c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return SYJSON_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = SYJSON_FALSE;
	return SYJSON_PARSE_OK;
}
//重构三种字面量类型解析
static int syjson_parse_literal(syjson_content* c, syjson_value* v, const char* literal, syjson_type type)
{
	size_t i;
	EXPECT(c, literal[0]);
	for(i=0; literal[i+1]; i++)
		if(c->json[i] != literal[i+1])
			return SYJSON_PARSE_INVALID_VALUE;
	c->json += i;
	v->type = type;
	return SYJSON_PARSE_OK;
}
//解析数字
#define ISDIGIT(string) ((string) >= '0' && (string) <= '9')
#define ISDIGIT1TO9(string) ((string) >= '1' && (string) <= '9')
static int syjson_parse_number(syjson_content* c, syjson_value* v)
{
	const char* p = c->json;
	//负号
	if(*p == '-') p++;
	//整数
	if(*p == '0') p++;
	else
	{
		if(!ISDIGIT1TO9(*p)) return SYJSON_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	//小数
	if(*p == '.')
	{
		p++;
		if(!ISDIGIT(*p)) return SYJSON_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	//指数
	if(*p == 'e' || *p == 'E')
	{
		p++;
		if(*p == '-' || *p == '+') p++;
		if(!ISDIGIT(*p)) return SYJSON_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}

	//函数校验数字真实性
	v->num = strtod(c->json, NULL);
	if(errno = ERANGE && (v->num == HUGE_VAL || v->num == -HUGE_VAL))
		return SYJSON_PARSE_NUMBER_TOO_BIG;
	c->json = p;
	v->type = SYJSON_NUM;
	return SYJSON_PARSE_OK;
}

//公共函数库
//值之后空白
static int syjson_parse_root_not_singular(syjson_content* c, syjson_value* v)
{
	syjson_parse_whitespace(c);
	if(c->json[0] == '\0')
		return SYJSON_PARSE_OK;
	else
	{
		v->type = SYJSON_NULL;
		return SYJSON_PARSE_ROOT_NOT_SINGULAR;
	}
}
//解析json入口
static int syjson_parse_value(syjson_content* c, syjson_value* v)
{
	switch(c->json[0])
	{
		case 'n': return syjson_parse_literal(c, v, "null", SYJSON_NULL);
		case 'f': return syjson_parse_literal(c, v, "false", SYJSON_FALSE);
		case 't': return syjson_parse_literal(c, v, "true", SYJSON_TRUE);
		case '\0': return SYJSON_PARSE_EXPECT_VALUE;
		default: return syjson_parse_number(c, v);
	}
}
//json库入口
int syjson_parse(syjson_value* v, const char* json)
{
	syjson_content c;
	assert(v != NULL);
	int result;
	c.json = json;
	v->type = SYJSON_NULL;
	syjson_parse_whitespace(&c);
	result = syjson_parse_value(&c, v);
	if(SYJSON_PARSE_OK == result)
		return syjson_parse_root_not_singular(&c, v);
	else
		return result;
}
//返回json数据类型
syjson_type syjson_get_type(const syjson_value* v)
{
	assert(v != NULL);
	return v->type;
}
//返回数字类型
double syjson_get_num(const syjson_value* v)
{
	assert(v != NULL && v->type == SYJSON_NUM);
	return v->num;
}
