#include <assert.h> /* assert() */
#include <errno.h>  /* errno, ERANGE */
#include <stdlib.h> /* NULL, malloc(), realloc(), free(), strtod() */
#include <math.h>   /* HUGE_VAL */
#include <string.h> /* memcpy() */

#include "syjson.h"

#define EXPECT(c, ch)  do{ assert(*c->json == (ch)); c->json++; }while(0)

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
	v->val.num = strtod(c->json, NULL);
	if(errno = ERANGE && (v->val.num == HUGE_VAL || v->val.num == -HUGE_VAL))
		return SYJSON_PARSE_NUMBER_TOO_BIG;
	c->json = p;
	v->type = SYJSON_NUM;
	return SYJSON_PARSE_OK;
}
//释放变量字符串空间 -- 也可以是 -- 初始化变量 set_null
void syjson_free(syjson_value* v)
{
	assert(v != NULL);
	if(v->type == SYJSON_STR) free(v->val.str.s);
	v->type = SYJSON_NULL;
}
//设置为真假
void syjson_set_boolean(syjson_value* v, int boolean)
{
	assert(v != NULL && (boolean == SYJSON_TRUE || boolean == SYJSON_FALSE));
	if(v->type == SYJSON_STR) free(v->val.str.s);
	v->type = boolean;
}
//设置变量数字
void syjson_set_number(syjson_value* v, double num)
{
	assert(v != NULL);
	if(v->type == SYJSON_STR) free(v->val.str.s);
	v->type = SYJSON_NUM;
	v->val.num = num;
}
//从字符串栈写入值空间
void syjson_set_string(syjson_value* v, const char* s, size_t len)
{
	assert(v != NULL && (s != NULL || len == 0));
	syjson_free(v);
	v->val.str.s = (char*)malloc(len + 1);
	memcpy(v->val.str.s, s, len);
	v->val.str.s[len] = '\0';
	v->val.str.l = len;
	v->type = SYJSON_STR;
}
//默认栈大小
#ifndef SYJSON_PARSE_STACK_INIT_SIZE
#define SYJSON_PARSE_STACK_INIT_SIZE 256
#endif
//压栈
static void* syjson_content_push(syjson_content* c, size_t size)
{
	void* ret;
	assert(size > 0);
	//检查是否超出栈空间
	if(c->top + size >= c->size)
	{
		//初始化栈空间
		if(c->size == 0)
			c->size = SYJSON_PARSE_STACK_INIT_SIZE;
		//以一点五倍增量增加栈空间
		while(c->top + size >= c->size)
			c->size += c->size >> 1;
		//申请堆内存，制作栈空间
		c->stack = (char*) realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += size;
	//返回栈顶地址
	return ret;
}
//出栈
static void* syjson_content_pop(syjson_content* c, size_t len)
{
	assert(c->top >= len);
	//这里的len为什么要跟head互换，直接传入head数值不行么
	return c->stack + (c->top -= len);
}
//压栈操作，字符写入到新地址
#define PUTC(c, ch) do{ *(char*)syjson_content_push(c, sizeof(char)) = (ch); }while(0)
//解析字符串错误
#define STRING_ERROR(ret) do{ c->top = head; return ret; }while(0)
//解析JSON转码至UNICODE码点，int4字节存储
static const char* syjson_parse_hex4(const char* p, unsigned* u)
{
	int i;
	*u = 0;
	//UNICODE基本面码点
	for(i = 0;i < 4;i++)
	{
		char ch = *p++;
		//16进制对应4位二进制
		*u <<= 4;
		//按位或写入数据，区分大小写
		if      (ch >= '0' && ch <= '9') *u |= ch - '0';
		//16进制，加上前缀十进制
		else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}
//UNICODE转至UTF-8编码格式
static void syjson_encode_utf8(syjson_content* c, unsigned u)
{
	//unicode码点，右移位数请查阅UTF-8编码实现
	if(u <= 0x007f)
		PUTC(c, u & 0xff);
	else if(u <= 0x07ff)
	{
		PUTC(c, ((u >> 6) & 0x1f) | 0xc0);
		PUTC(c, ( u       & 0x3f) | 0x80);
	}
	else if(u <= 0xffff)
	{
		PUTC(c, ((u >> 12) & 0x0f) | 0xe0);
		PUTC(c, ((u >>  6) & 0x3f) | 0x80);
		PUTC(c, ( u        & 0x3f) | 0x80);
	}
	else if(u <= 0x10ffff)
	{
		PUTC(c, ((u >> 18) & 0x08) | 0xf0);
		PUTC(c, ((u >> 12) & 0x3f) | 0x80);
		PUTC(c, ((u >> 6)  & 0x3f) | 0x80);
		PUTC(c, ( u        & 0x3f) | 0x80);
	}
}
//解析字符串
static int syjson_parse_string(syjson_content* c, syjson_value* v)
{
	size_t head = c->top, len;
	unsigned u, u2;
	const char* p;
	//检查字符串初始字符，并向后位移指针
	EXPECT(c, '\"');
	p = c->json;
	for(;;)
	{
		char ch = *p++;
		switch(ch)
		{
			//解析字符串结束，获取字符串长度，批量写入字符空间
			case '\"':
				len = c->top - head;
				syjson_set_string(v, (const char*)syjson_content_pop(c, len), len);
				c->json = p;
				return SYJSON_PARSE_OK;
			//解析转义
			case '\\':
				switch(*p++)
				{
					//转义字符压栈
					case '\"': PUTC(c, '\"'); break;
					case '\\': PUTC(c, '\\'); break;
					case '/' : PUTC(c, '/' ); break;
					case 'b' : PUTC(c, '\b'); break;
					case 'f' : PUTC(c, '\f'); break;
					case 'n' : PUTC(c, '\n'); break;
					case 'r' : PUTC(c, '\r'); break;
					case 't' : PUTC(c, '\t'); break;
					//解析UTF-8
					case 'u':
						//验证进制合法，并前移字符指针
						if(!(p = syjson_parse_hex4(p, &u)))
							STRING_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX);
						//高代理码点
						if(u >= 0xd800 && u <= 0xdbff)
						{
							if(*p++ != '\\')
								STRING_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE);
							if(*p++ != 'u')
								STRING_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE);
							if(!(p = syjson_parse_hex4(p, &u2)))
								STRING_ERROR(SYJSON_PARSE_INVALID_UNICODE_HEX);
							//低代理码点
							if(u2 < 0xdc00 || u2 > 0xdfff)
								STRING_ERROR(SYJSON_PARSE_INVALID_UNICODE_SURROGATE);
							//辅助平面码点合并，0x10000 至 0x10ffff，总共20bit数据，高位10bit，低位10bit
							u =  0x10000 + (((u - 0xd800) << 10) | (u2 - 0xdc00));
						}
						//编码为UTF-8，并压栈
						syjson_encode_utf8(c, u);
					break;
					default:
						c->top = head;
						STRING_ERROR(SYJSON_PARSE_INVALID_STRING_ESCAPE);
				}
				break;
			case '\0':
				c->top = head;
				return SYJSON_PARSE_MISS_QUOTATION_MARK;
				break;
			default:
				if((unsigned char)ch < 0x20)
				{
					c->top = head;
					return SYJSON_PARSE_INVALID_STRING_CHAR;
				}
				PUTC(c, ch);
		}
	}
}
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
		case '"': return syjson_parse_string(c, v);
		default: return syjson_parse_number(c, v);
	}
}

//json库入口
int syjson_parse(syjson_value* v, const char* json)
{
	syjson_content c;
	int result;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	syjson_init(v);
	v->type = SYJSON_NULL;
	syjson_parse_whitespace(&c);
	result = syjson_parse_value(&c, v);
	if(SYJSON_PARSE_OK == result)
		result = syjson_parse_root_not_singular(&c, v);

	assert(c.top == 0);
	free(c.stack);
	return result;
}
//返回json数据类型
syjson_type syjson_get_type(const syjson_value* v)
{
	assert(v != NULL);
	return v->type;
}
//返回真假值，并非变量类型
int syjson_get_boolean(const syjson_value* v)
{
	assert(v != NULL && (v->type == SYJSON_TRUE || v->type == SYJSON_FALSE));
	return v->type == SYJSON_TRUE;
}
//返回数字类型
double syjson_get_number(const syjson_value* v)
{
	assert(v != NULL && v->type == SYJSON_NUM);
	return v->val.num;
}
//返回字符串长度
size_t syjson_get_string_length(const syjson_value* v)
{
	assert(v != NULL && v->type == SYJSON_STR);
	return v->val.str.l;
}
//返回字符串
const char* syjson_get_string(const syjson_value* v)
{
	assert(v != NULL && v->type == SYJSON_STR);
	return v->val.str.s;
}








