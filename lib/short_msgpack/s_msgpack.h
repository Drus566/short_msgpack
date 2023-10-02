#ifndef S_MSGPACK_H
#define S_MSGPACK_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum
{
	S_MSGPACK_TYPE_EMPTY = 0,
	S_MSGPACK_TYPE_POSITIVE_FIXINT = 1,
	S_MSGPACK_TYPE_NEGATIVE_FIXINT,
	S_MSGPACK_TYPE_UINT8,
	S_MSGPACK_TYPE_UINT16,
	S_MSGPACK_TYPE_UINT32,
	S_MSGPACK_TYPE_UINT64,
	S_MSGPACK_TYPE_INT8,
	S_MSGPACK_TYPE_INT16,
	S_MSGPACK_TYPE_INT32,
	S_MSGPACK_TYPE_INT64,
	S_MSGPACK_TYPE_FLOAT32,
	S_MSGPACK_TYPE_FLOAT64,
	S_MSGPACK_TYPE_FIXSTR,
	S_MSGPACK_TYPE_STR8,
	S_MSGPACK_TYPE_STR16,
	S_MSGPACK_TYPE_STR32,
	S_MSGPACK_TYPE_FIXARRAY,
	S_MSGPACK_TYPE_ARRAY16,
	S_MSGPACK_TYPE_ARRAY32,
	S_MSGPACK_TYPE_FIXMAP,
	S_MSGPACK_TYPE_MAP16,
	S_MSGPACK_TYPE_MAP32,
	S_MSGPACK_TYPE_NIL,
	S_MSGPACK_TYPE_NEVERUSED,
	S_MSGPACK_TYPE_TRUE,
	S_MSGPACK_TYPE_FALSE,
	S_MSGPACK_TYPE_BIN8,
	S_MSGPACK_TYPE_BIN16,
	S_MSGPACK_TYPE_BIN32,
	S_MSGPACK_TYPE_EXT8,
	S_MSGPACK_TYPE_EXT16,
	S_MSGPACK_TYPE_EXT32,
	S_MSGPACK_TYPE_FIXEXT1,
	S_MSGPACK_TYPE_FIXEXT2,
	S_MSGPACK_TYPE_FIXEXT4,
	S_MSGPACK_TYPE_FIXEXT8,
	S_MSGPACK_TYPE_FIXEXT16,
	S_MSGPACK_TYPE_STR,
	S_MSGPACK_TYPE_INT,
	S_MSGPACK_TYPE_UINT,
	S_MSGPACK_TYPE_FLOAT,
	S_MSGPACK_TYPE_BOOL,
	S_MSGPACK_TYPE_ARRAY,
	S_MSGPACK_TYPE_MAP
};

typedef struct s_msgpack_t s_msgpack_t;
struct s_msgpack_t
{
	char *read_buf_start;
	char *read_buf_current;
	uint32_t read_buf_size;
	uint32_t read_buf_pos;

	char *write_buf_start;
	char *write_buf_current;
	uint32_t write_buf_size;
	uint32_t write_buf_pos;

	union
	{
		uint8_t ui8;
		uint16_t ui16;
		uint32_t ui32;
		int8_t i8;
		int16_t i16;
		int32_t i32;
		float f;
	} obj;
};

/* Инициализация, параметры...
	1. структура
	2. откуда будем читать
	3. размер буфера откуда читаем
	4. куда будем писать
	5. размер буфера куда пишем
*/
void s_msgpack_init(s_msgpack_t *msgpack,
						  char *read_buf,
						  uint32_t read_buf_size,
						  char *write_buf,
						  uint32_t write_buf_size);

/* Получить тип элемента сообщения msgpack в виде строки */
uint8_t s_msgpack_type_to_str(uint8_t header, char *str, uint8_t str_size);

/* Прочитать int (любой целочисленный знаковый тип, кроме int64) тип */
uint8_t s_msgpack_int_read(s_msgpack_t *msgpack, int32_t *i);
/* Прочитать uint (любой целочисленный беззнаковый тип, кроме uint64) тип */
uint8_t s_msgpack_uint_read(s_msgpack_t *msgpack, uint32_t *i);
/* Прочитать positive_fixint тип */
uint8_t s_msgpack_positive_fixint_read(s_msgpack_t *msgpack, uint8_t *ui);
/* Прочитать negative_fixint тип */
uint8_t s_msgpack_negative_fixint_read(s_msgpack_t *msgpack, int8_t *i);
/* Прочитать int8 тип */
uint8_t s_msgpack_int8_read(s_msgpack_t *msgpack, int8_t *i);
/* Прочитать uint8 тип */
uint8_t s_msgpack_uint8_read(s_msgpack_t *msgpack, uint8_t *ui);
/* Прочитать int16 тип */
uint8_t s_msgpack_int16_read(s_msgpack_t *msgpack, int16_t *i);
/* Прочитать uint16 тип */
uint8_t s_msgpack_uint16_read(s_msgpack_t *msgpack, uint16_t *ui);
/* Прочитать int32 тип */
uint8_t s_msgpack_int32_read(s_msgpack_t *msgpack, int32_t *i);
/* Прочитать uint32 тип */
uint8_t s_msgpack_uint32_read(s_msgpack_t *msgpack, uint32_t *ui);
/* Прочитать float (любой тип с плавающий точкой, в нашем случае тип float32) тип */
uint8_t s_msgpack_float_read(s_msgpack_t *msgpack, float *f);
/* Прочитать float32 тип */
uint8_t s_msgpack_float32_read(s_msgpack_t *msgpack, float *f);
/* Прочитать str (любой строковый тип) */
uint32_t s_msgpack_str_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
/* Прочитать fixstr тип */
uint8_t s_msgpack_fixstr_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
/* Прочитать str8 тип */
uint8_t s_msgpack_str8_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
/* Прочитать str16 тип */
uint16_t s_msgpack_str16_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
/* Прочитать str32 тип */
uint32_t s_msgpack_str32_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
/* Прочитать array (любой элемент тип массива) тип */
uint32_t s_msgpack_array_read(s_msgpack_t *msgpack);
/* Прочитать fixarray тип */
uint8_t s_msgpack_fixarray_read(s_msgpack_t *msgpack);
/* Прочитать array16 тип */
uint16_t s_msgpack_array16_read(s_msgpack_t *msgpack);
/* Прочитать array32 тип */
uint32_t s_msgpack_array32_read(s_msgpack_t *msgpack);
/* Прочитать map (любой элемент тип хеш) тип */
uint32_t s_msgpack_map_read(s_msgpack_t *msgpack);
/* Прочитать fixmap тип */
uint8_t s_msgpack_fixmap_read(s_msgpack_t *msgpack);
/* Прочитать map16 тип */
uint16_t s_msgpack_map16_read(s_msgpack_t *msgpack);
/* Прочитать map32 тип */
uint32_t s_msgpack_map32_read(s_msgpack_t *msgpack);
/* Прочитать bool (любой булевый тип, true или false) */
uint8_t s_msgpack_bool_read(s_msgpack_t *msgpack, uint8_t *b);
/* Прочитать true тип */
uint8_t s_msgpack_true_read(s_msgpack_t *msgpack);
/* Прочитать false тип */
uint8_t s_msgpack_false_read(s_msgpack_t *msgpack);
/* Прочитать nil тип */
uint8_t s_msgpack_nil_read(s_msgpack_t *msgpack);
/* Прочитать neverused тип */
uint8_t s_msgpack_neverused_read(s_msgpack_t *msgpack);
/* Вернуть тип N-ого элемента */
uint8_t s_msgpack_read_elem(s_msgpack_t *msgpack, uint32_t elem_number);

/* Записать знаковый целочисленный тип */
uint8_t s_msgpack_int_write(s_msgpack_t *msgpack, int32_t i);
/* Записать беззнаковый целочисленный тип */
uint8_t s_msgpack_uint_write(s_msgpack_t *msgpack, uint32_t i);
/* Записать positive_fixint тип */
uint8_t s_msgpack_positive_fixint_write(s_msgpack_t *msgpack, uint8_t ui);
/* Записать negative_fixint тип */
uint8_t s_msgpack_negative_fixint_write(s_msgpack_t *msgpack, int8_t i);
/* Записать int8 тип */
uint8_t s_msgpack_int8_write(s_msgpack_t *msgpack, int8_t i);
/* Записать uint8 тип */
uint8_t s_msgpack_uint8_write(s_msgpack_t *msgpack, uint8_t ui);
/* Записать int16 тип */
uint8_t s_msgpack_int16_write(s_msgpack_t *msgpack, int16_t i);
/* Записать uint16 тип */
uint8_t s_msgpack_uint16_write(s_msgpack_t *msgpack, uint16_t ui);
/* Записать int32 тип */
uint8_t s_msgpack_int32_write(s_msgpack_t *msgpack, int32_t i);
/* Записать uint32 тип */
uint8_t s_msgpack_uint32_write(s_msgpack_t *msgpack, uint32_t ui);
/* Записать float тип */
uint8_t s_msgpack_float_write(s_msgpack_t *msgpack, float f);
/* Записать float32 тип */
uint8_t s_msgpack_float32_write(s_msgpack_t *msgpack, float f);
/* Записать str тип */
uint8_t s_msgpack_str_write(s_msgpack_t *msgpack, char *s, uint32_t size);
/* Записать fixstr тип */
uint8_t s_msgpack_fixstr_write(s_msgpack_t *msgpack, char *s, uint8_t size);
/* Записать str8 тип */
uint8_t s_msgpack_str8_write(s_msgpack_t *msgpack, char *s, uint8_t size);
/* Записать str16 тип */
uint16_t s_msgpack_str16_write(s_msgpack_t *msgpack, char *s, uint16_t size);
/* Записать str32 тип */
uint32_t s_msgpack_str32_write(s_msgpack_t *msgpack, char *s, uint32_t size);
/* Записать array тип */
uint8_t s_msgpack_array_write(s_msgpack_t *msgpack, uint32_t size);
/* Записать fixarray тип */
uint8_t s_msgpack_fixarray_write(s_msgpack_t *msgpack, uint8_t size);
/* Записать array16 тип */
uint8_t s_msgpack_array16_write(s_msgpack_t *msgpack, uint16_t size);
/* Записать array32 тип */
uint8_t s_msgpack_array32_write(s_msgpack_t *msgpack, uint32_t size);
/* Записать map тип */
uint8_t s_msgpack_map_write(s_msgpack_t *msgpack, uint32_t size);
/* Записать fixmap тип */
uint8_t s_msgpack_fixmap_write(s_msgpack_t *msgpack, uint8_t size);
/* Записать map16 тип */
uint8_t s_msgpack_map16_write(s_msgpack_t *msgpack, uint16_t size);
/* Записать map32 тип */
uint8_t s_msgpack_map32_write(s_msgpack_t *msgpack, uint32_t size);
/* Записать bool тип */
uint8_t s_msgpack_bool_write(s_msgpack_t *msgpack, uint8_t b);
/* Записать true тип */
uint8_t s_msgpack_true_write(s_msgpack_t *msgpack);
/* Записать false тип */
uint8_t s_msgpack_false_write(s_msgpack_t *msgpack);
/* Записать nil тип */
uint8_t s_msgpack_nil_write(s_msgpack_t *msgpack);
/* Записать neverused тип */
uint8_t s_msgpack_neverused_write(s_msgpack_t *msgpack);

/* Является ли заголовок типа массивом */
uint8_t s_msgpack_is_array(uint8_t header);
/* Является ли заголовок типа строкой */
uint8_t s_msgpack_is_str(uint8_t header);
/* Является ли заголовок типа булевым */
uint8_t s_msgpack_is_bool(uint8_t header);
/* Является ли заголовок типа числовым (любые числовые в том числе с плавающей запятой) */
uint8_t s_msgpack_is_numeric(uint8_t header);
/* Является ли заголовок типа знаковым целочисленным */
uint8_t s_msgpack_is_int(uint8_t header);
/* Является ли заголовок типа беззнаковым целочисленным */
uint8_t s_msgpack_is_uint(uint8_t header);
/* Является ли заголовок типа с плавающей запятой */
uint8_t s_msgpack_is_float(uint8_t header);

/*** Операции с буферами записи и чтения ***/

/* Установить новый буфер для чтения */
void s_msgpack_set_read_buf(s_msgpack_t *msgpack, char *buf, uint32_t bufsize);
/* Установить буфер чтения в позицию начала */
void s_msgpack_set_to_start_read_buf(s_msgpack_t *msgpack);
/* Установить буфер чтения в указанную позицию */
uint8_t s_msgpack_set_to_pos_read_buf(s_msgpack_t *msgpack, uint32_t pos);
/* Установить позицию буфера чтения на N-ый элемент */
uint8_t s_msgpack_set_read_buf_pos_to_elem(s_msgpack_t *msgpack, uint32_t elem);
/* Получить позицию буфера чтения */
uint32_t s_msgpack_get_read_buf_pos(s_msgpack_t *msgpack);
/* Получить заголовок типа на текущей позиции буфера чтения */
uint8_t s_msgpack_get_read_buf_header(s_msgpack_t *msgpack);
/* Получить позицию буфера на N-ом элементе */
uint32_t s_msgpack_get_read_buf_pos_elem(s_msgpack_t *msgpack, uint32_t elem);
/* Получить длинну буфера чтения */
uint32_t s_msgpack_get_read_buf_length(s_msgpack_t *msgpack);
/* Получить буфер чтения */
char* s_msgpack_get_read_buf(s_msgpack_t *msgpack);
/* Получить текущий байт буфера чтения */
unsigned char s_msgpack_get_read_buf_byte(s_msgpack_t *msgpack);
/* Получить байт буфера записи на указанной позиции */
unsigned char s_msgpack_get_write_buf_pos_byte(s_msgpack_t *msgpack, uint32_t pos);

/* Установить новый буфер для записи */
void s_msgpack_set_write_buf(s_msgpack_t *msgpack, char *buf, uint32_t bufsize);
/* Установить буфер записи в позицию начала */
void s_msgpack_set_to_start_write_buf(s_msgpack_t *msgpack);
/* Установить буфер записи в указанную позицию */
uint8_t s_msgpack_set_to_pos_write_buf(s_msgpack_t *msgpack, uint32_t pos);
/* Получить позицию буфера записи */
uint32_t s_msgpack_get_write_buf_pos(s_msgpack_t *msgpack);
/* Получить заголовок типа на текущей позиции буфера записи */
uint8_t s_msgpack_get_write_buf_header(s_msgpack_t *msgpack);
/* Получить длинну буфера записи */
uint32_t s_msgpack_get_write_buf_length(s_msgpack_t *msgpack);
/* Получить буфер записи */
char* s_msgpack_get_write_buf(s_msgpack_t *msgpack);
/* Получить текущий байт буфера записи */
unsigned char s_msgpack_get_write_buf_byte(s_msgpack_t *msgpack);
/* Получить байт буфера записи на указанной позиции */
unsigned char s_msgpack_get_write_buf_pos_byte(s_msgpack_t *msgpack, uint32_t pos);

/* Распарсить буфер чтения в указанную строку */
uint8_t s_msgpack_read_to_str(s_msgpack_t *msgpack, char *str, const uint64_t str_size);

#endif // S_MSGPACK_H
