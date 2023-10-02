#include "s_msgpack.h"

/*************************************** Заметки *****************************************/

/* Ключом в хеше обычно выступает строка  */
/* После каждой записи или чтения идет переход на следующую позицию байта,
	если эта позиция выходит за рамки размера буфера,
	то любая функция чтения или записи вернет ALARM,
	даже если чтение или запись была успешна
*/

/*****************************************************************************************/

/********************************** Вспомогательные **************************************/

/* Маска на старшие 4 бита - 1111 0000 */
#define MSB_MASK 0xf0
/* Маска на младшие 4 бита - 0000 1111 */
#define LSB_MASK 0xf
/* Максимальное количество вложенных структур */
// #define NESTING_MAX 8
/* Маска байта */
#define BYTE_MASK 0xff

/* Максимальное количество элементов в массиве */
#define ARRAY_SIZE 256
/* Максимальное количество пар ключ\значение в хеше */
#define MAP_SIZE 256

/* Булевы и другие значения */
#define TRUE 1
#define FALSE 0
#define NIL 0
#define NEVER_USED 0

/* Для отслеживания успешно ли было чтение */
#define OK 1
#define ALARM 0

/*****************************************************************************************/

/***************************** Фиксированные положительные целые *************************/

/* идентификатор */
#define POSITIVE_FIXINT_ID 0x00 // от 0 до 0x7f (dec 127)
/* маска идентификатора */
#define POSITIVE_FIXINT_ID_MASK 0x80
/* маска числа */
#define POSITIVE_FIXINT_NUMBER_MASK 0x7f
/* максимальное число */
#define POSITIVE_FIXINT_NUMBER_MAX 0x7f
/* минимальное число */
#define POSITIVE_FIXINT_NUMBER_MIN 0x00

/*****************************************************************************************/

/***************************** Фиксированные отрицательные целые *************************/

/* идентификатор */
#define NEGATIVE_FIXINT_ID 0xe0 // от -1 до -32
/* маска идентификатора */
#define NEGATIVE_FIXINT_ID_MASK 0xe0
/* маска числа */
#define NEGATIVE_FIXINT_NUMBER_MASK 0x1f
/* максимальное число */
#define NEGATIVE_FIXINT_NUMBER_MAX -1
/* минимальное число */
#define NEGATIVE_FIXINT_NUMBER_MIN -32

/*****************************************************************************************/

/************************************ Целые без знака ************************************/

/* идентификатор */
#define UINT8_ID 0xcc
/* длинна данных в байтах */
#define UINT8_DATA_SIZE 1
/* максимальное число */
#define UINT8_NUMBER_MAX 0xff
/* минимальное число */
#define UINT8_NUMBER_MIN 0x00

/* идентификатор */
#define UINT16_ID 0xcd
/* длинна данных в байтах */
#define UINT16_DATA_SIZE 2
/* максимальное число */
#define UINT16_NUMBER_MAX 0xffff
/* минимальное число */
#define UINT16_NUMBER_MIN 0x00

/* идентификатор */
#define UINT32_ID 0xce
/* длинна данных в байтах */
#define UINT32_DATA_SIZE 4
/* максимальное число */
#define UINT32_NUMBER_MAX 0xffffffff
/* минимальное число */
#define UINT32_NUMBER_MIN 0x00

/*****************************************************************************************/

/************************************ Целые со знаком ************************************/

/* идентификатор */
#define INT8_ID 0xd0
/* длинна данных в байтах */
#define INT8_DATA_SIZE 1
/* максимальное число */
#define INT8_NUMBER_MAX 0x7f
/* минимальное число */
#define INT8_NUMBER_MIN -128

/* идентификатор */
#define INT16_ID 0xd1
/* длинна данных в байтах */
#define INT16_DATA_SIZE 2
/* максимальное число */
#define INT16_NUMBER_MAX 32767
/* минимальное число */
#define INT16_NUMBER_MIN -32768

/* идентификатор */
#define INT32_ID 0xd2
/* длинна данных в байтах */
#define INT32_DATA_SIZE 4
/* максимальное число */
#define INT32_NUMBER_MAX 2147483647
/* минимальное число */
#define INT32_NUMBER_MIN -2147483648

/*****************************************************************************************/

/***************************** Число с плавающей точкой **********************************/

/* идентификатор */
#define FLOAT32_ID 0xca
/* длинна данных в байтах */
#define FLOAT32_DATA_SIZE 4
/* максимальное число */
#define FLOAT32_NUMBER_MAX 3.4e+38
/* минимальное число */
#define FLOAT32_NUMBER_MIN -3.4e+38

/*****************************************************************************************/

/**************************** Фиксированная строка, макс. размер - 31 ********************/

/* идентификатор */
#define FIXSTR_ID 0xa0 // до 0xbf
/* маска идентификатора */
#define FIXSTR_ID_MASK 0xe0
/* маска количества байтов в строке */
#define FIXSTR_NUMBER_MASK 0x1f
/* максимальное количество байтов в строке */
#define FIXSTR_NUMBER_MAX 0x1f

/*****************************************************************************************/

/********************************* Строки с переменной длинной ***************************/

/* идентификатор */
#define STR8_ID 0xd9
/* длинна в байтах поля с количеством элементов массива */
#define STR8_LENGTH_SIZE 1
/* максимальное количество байтов в строке */
#define STR8_NUMBER_MAX 0xff

/* идентификатор */
#define STR16_ID 0xda
/* длинна в байтах поля с количеством элементов массива */
#define STR16_LENGTH_SIZE 2
/* максимальное количество байтов в строке */
#define STR16_NUMBER_MAX 0xffff

/* идентификатор */
#define STR32_ID 0xdb
/* длинна в байтах поля с количеством элементов массива */
#define STR32_LENGTH_SIZE 4
/* максимальное количество байтов в строке */
#define STR32_NUMBER_MAX 0xffffffff

/*****************************************************************************************/

/*** Фиксированный массив, например [1,2,"str"], макс. кол-во элементов в массиве - 15 ***/

/* идентификатор */
#define FIXARRAY_ID 0x90 // до 0x9f
/* маска идентификтора */
#define FIXARRAY_ID_MASK MSB_MASK
/* маска количества элементов в массиве */
#define FIXARRAY_NUMBER_MASK LSB_MASK
/* максимальное количество элементов в массиве */
#define FIXARRAY_NUMBER_MAX 0x0f

/*****************************************************************************************/

/******************************* Массив с переменной длинной *****************************/

/* идентификатор */
#define ARRAY16_ID 0xdc
/* длинна в байтах поля с количеством элементов массива */
#define ARRAY16_LENGTH_SIZE 2
/* максимальное количество элементов в массиве */
#define ARRAY16_NUMBER_MAX 0xffff

/* идентификатор */
#define ARRAY32_ID 0xdd
/* длинна в байтах поля с количеством элементов массива */
#define ARRAY32_LENGTH_SIZE 4
/* максимальное количество элементов в массиве */
#define ARRAY32_NUMBER_MAX 0xffffffff

/*****************************************************************************************/

/**** Хеш дерево, например {"str":1,"foo":"bar"}, макс. кол-во элементов в хеше - 15 *****/

/* идентификатор */
#define FIXMAP_ID 0x80
/* маска идентификатора */
#define FIXMAP_ID_MASK 0xf0
/* маска количества хешей  */
#define FIXMAP_NUMBER_MASK 0xf
/* максимальное количество хешей */
#define FIXMAP_NUMBER_MAX 0x8f

/*****************************************************************************************/

/********************************* Хэш с переменной длинной ******************************/

/* идентификатор */
#define MAP16_ID 0xde
/* длинна в байтах поля с количеством элементов массива */
#define MAP16_LENGTH_SIZE 2
/* максимальное количество хешей */
#define MAP16_NUMBER_MAX 0xffff

/* идентификатор */
#define MAP32_ID 0xdf
/* длинна в байтах поля с количеством элементов массива */
#define MAP32_LENGTH_SIZE 4
/* максимальное количество хешей */
#define MAP32_NUMBER_MAX 0xffffffff

/*****************************************************************************************/

/*********************************** NULL и NEVER USED значения **************************/

#define NIL_ID 0xc0
#define NEVER_USED_ID 0xc1

/*****************************************************************************************/

/*************************************** Булевы значения *********************************/

#define FALSE_ID 0xc2
#define TRUE_ID 0xc3

/*****************************************************************************************/

#define FB_BIN8 0xc4
#define FB_BIN16 0xc5
#define FB_BIN32 0xc9
#define FB_EXT8 0xc7
#define FB_EXT16 0xc8
#define FB_EXT32 0xc9
#define FB_FIXEXT1 0xd4
#define FB_FIXEXT2 0xd5
#define FB_FIXEXT4 0xd6
#define FB_FIXEXT8 0xd7
#define FB_FIXEXT16 0xd8

static uint8_t check_header(char byte);

static uint8_t positive_fixint_read_handler(s_msgpack_t *msgpack);
static uint8_t negative_fixint_read_handler(s_msgpack_t *msgpack);
static uint8_t uint8_read_handler(s_msgpack_t *msgpack);
static uint8_t uint16_read_handler(s_msgpack_t *msgpack);
static uint8_t uint32_read_handler(s_msgpack_t *msgpack);
static uint8_t int8_read_handler(s_msgpack_t *msgpack);
static uint8_t int16_read_handler(s_msgpack_t *msgpack);
static uint8_t int32_read_handler(s_msgpack_t *msgpack);
static uint8_t float32_read_handler(s_msgpack_t *msgpack);
static uint8_t fixstr_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
static uint8_t str8_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
static uint16_t str16_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
static uint32_t str32_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize);
static uint8_t fixarray_read_handler(s_msgpack_t *msgpack);
static uint16_t array16_read_handler(s_msgpack_t *msgpack);
static uint32_t array32_read_handler(s_msgpack_t *msgpack);
static uint8_t fixmap_read_handler(s_msgpack_t *msgpack);
static uint16_t map16_read_handler(s_msgpack_t *msgpack);
static uint32_t map32_read_handler(s_msgpack_t *msgpack);
static uint8_t true_read_handler(s_msgpack_t *msgpack);
static uint8_t false_read_handler(s_msgpack_t *msgpack);
static uint8_t nil_read_handler(s_msgpack_t *msgpack);
static uint8_t neverused_read_handler(s_msgpack_t *msgpack);

static uint32_t read_elem_header_pos(s_msgpack_t *msgpack, uint32_t elem_number, uint8_t pos_mode);

static void swap_byte16(char *number_ptr, char *msgpack);
static void swap_byte32(char *number_ptr, char *msgpack);
static void swap_byte64(char *number_ptr, char *msgpack);

static uint8_t read_buf_shift_pos(s_msgpack_t *msgpack, uint32_t shift);
static uint8_t read_buf_greather_limit(s_msgpack_t *msgpack, uint32_t shift);
static uint8_t write_buf_shift_pos(s_msgpack_t *msgpack, uint32_t shift);
static uint8_t write_buf_greather_limit(s_msgpack_t *msgpack, uint32_t shift);

static uint8_t start_read_to_str(s_msgpack_t *msgpack, char *str, const uint64_t str_size, uint64_t *cur_size);
static uint8_t cpy_msg_to_str(char *str, uint64_t str_size, uint64_t *cur_size, char *msg, uint32_t msg_size);
static uint8_t process_read_array(s_msgpack_t *msgpack, uint32_t ar_size, char *str, uint64_t str_size, uint64_t *cur_size);
static uint8_t process_read_map(s_msgpack_t *msgpack, uint32_t ar_size, char *str, uint64_t str_size, uint64_t *cur_size);

void s_msgpack_init(s_msgpack_t *msgpack,
						  char *read_buf,
						  uint32_t read_buf_size,
						  char *write_buf,
						  uint32_t write_buf_size)
{
	msgpack->read_buf_start = read_buf;
	msgpack->read_buf_current = msgpack->read_buf_start;
	msgpack->read_buf_size = read_buf_size;
	msgpack->read_buf_pos = 0;

	msgpack->write_buf_start = write_buf;
	msgpack->write_buf_current = msgpack->write_buf_start;
	msgpack->write_buf_size = write_buf_size;
	msgpack->write_buf_pos = 0;
}

static uint8_t check_header(char byte)
{
	uint8_t header = S_MSGPACK_TYPE_EMPTY;

	if ((byte & POSITIVE_FIXINT_ID_MASK) == POSITIVE_FIXINT_ID)
	{
		header = S_MSGPACK_TYPE_POSITIVE_FIXINT;
	}
	else if ((byte & NEGATIVE_FIXINT_ID_MASK) == NEGATIVE_FIXINT_ID)
	{
		header = S_MSGPACK_TYPE_NEGATIVE_FIXINT;
	}
	else if ((byte & BYTE_MASK) == UINT8_ID)
	{
		header = S_MSGPACK_TYPE_UINT8;
	}
	else if ((byte & BYTE_MASK) == UINT16_ID)
	{
		header = S_MSGPACK_TYPE_UINT16;
	}
	else if ((byte & BYTE_MASK) == UINT32_ID)
	{
		header = S_MSGPACK_TYPE_UINT32;
	}
	else if ((byte & BYTE_MASK) == INT8_ID)
	{
		header = S_MSGPACK_TYPE_INT8;
	}
	else if ((byte & BYTE_MASK) == INT16_ID)
	{
		header = S_MSGPACK_TYPE_INT16;
	}
	else if ((byte & BYTE_MASK) == INT32_ID)
	{
		header = S_MSGPACK_TYPE_INT32;
	}
	else if ((byte & BYTE_MASK) == FLOAT32_ID)
	{
		header = S_MSGPACK_TYPE_FLOAT32;
	}
	else if ((byte & FIXSTR_ID_MASK) == FIXSTR_ID)
	{
		header = S_MSGPACK_TYPE_FIXSTR;
	}
	else if ((byte & BYTE_MASK) == STR8_ID)
	{
		header = S_MSGPACK_TYPE_STR8;
	}
	else if ((byte & BYTE_MASK) == STR16_ID)
	{
		header = S_MSGPACK_TYPE_STR16;
	}
	else if ((byte & BYTE_MASK) == STR32_ID)
	{
		header = S_MSGPACK_TYPE_STR32;
	}
	else if ((byte & FIXARRAY_ID_MASK) == FIXARRAY_ID)
	{
		header = S_MSGPACK_TYPE_FIXARRAY;
	}
	else if ((byte & BYTE_MASK) == ARRAY16_ID)
	{
		header = S_MSGPACK_TYPE_ARRAY16;
	}
	else if ((byte & BYTE_MASK) == ARRAY32_ID)
	{
		header = S_MSGPACK_TYPE_ARRAY32;
	}
	else if ((byte & FIXMAP_ID_MASK) == FIXMAP_ID)
	{
		header = S_MSGPACK_TYPE_FIXMAP;
	}
	else if ((byte & BYTE_MASK) == MAP16_ID)
	{
		header = S_MSGPACK_TYPE_MAP16;
	}
	else if ((byte & BYTE_MASK) == MAP32_ID)
	{
		header = S_MSGPACK_TYPE_MAP32;
	}
	else if ((byte & BYTE_MASK) == TRUE_ID)
	{
		header = S_MSGPACK_TYPE_TRUE;
	}
	else if ((byte & BYTE_MASK) == FALSE_ID)
	{
		header = S_MSGPACK_TYPE_FALSE;
	}
	else if ((byte & BYTE_MASK) == NIL_ID)
	{
		header = S_MSGPACK_TYPE_NIL;
	}
	else if ((byte & BYTE_MASK) == NEVER_USED_ID)
	{
		header = S_MSGPACK_TYPE_NEVERUSED;
	}
	return header;
}

/************************************* HANDLERS ******************************************/

static uint8_t positive_fixint_read_handler(s_msgpack_t *msgpack)
{
	msgpack->obj.ui8 = *msgpack->read_buf_current & POSITIVE_FIXINT_NUMBER_MASK;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t negative_fixint_read_handler(s_msgpack_t *msgpack)
{
	msgpack->obj.i8 = *msgpack->read_buf_current;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t uint8_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	msgpack->obj.ui8 = *msgpack->read_buf_current;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t uint16_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 1))
	{
		return ALARM;
	}
	uint16_t number = 0;
	char *i_ptr = (char *)&number;

	swap_byte16(i_ptr, msgpack->read_buf_current);
	msgpack->obj.ui16 = number;

	if (!read_buf_shift_pos(msgpack, 2))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t uint32_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 3))
	{
		return ALARM;
	}
	uint32_t number = 0;
	char *i_ptr = (char *)&number;

	swap_byte32(i_ptr, msgpack->read_buf_current);
	msgpack->obj.ui32 = number;

	if (!read_buf_shift_pos(msgpack, 4))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t int8_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	msgpack->obj.i8 = *msgpack->read_buf_current;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t int16_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 1))
	{
		return ALARM;
	}

	swap_byte16((char *)&msgpack->obj.i16, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, 2))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t int32_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 3))
	{
		return ALARM;
	}

	swap_byte32((char *)&msgpack->obj.i32, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, 4))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t float32_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 3))
	{
		return ALARM;
	}

	swap_byte32((char *)&msgpack->obj.f, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, 4))
	{
		return ALARM;
	}

	return OK;
}

static uint8_t fixstr_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	uint8_t length = (*msgpack->read_buf_current & FIXSTR_NUMBER_MASK);
	uint8_t limit = length;
	if (bufsize < length)
	{
		limit = bufsize;
	}

	for (int i = 0; i < limit; i++)
	{
		if (!read_buf_shift_pos(msgpack, 1))
		{
			return ALARM;
		}
		*s++ = *msgpack->read_buf_current;
	}

	if (bufsize < length + 1)
	{
		printf("FIXSTR length error, string will be truncated\n");
		*--s = '\0';
	}
	else
	{
		*s = '\0';
	}

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return length;
}

static uint8_t str8_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}
	uint8_t length = *msgpack->read_buf_current;
	uint8_t limit = length;
	if (bufsize < length)
	{
		limit = bufsize;
	}

	for (int i = 0; i < limit; i++)
	{
		if (!read_buf_shift_pos(msgpack, 1))
		{
			return ALARM;
		}
		*s++ = *msgpack->read_buf_current;
	}

	if (bufsize < length + 1)
	{
		printf("STR8 length error, string will be truncated\n");
		*--s = '\0';
	}
	else
	{
		*s = '\0';
	}

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	};

	return length;
}

static uint16_t str16_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 1))
	{
		return ALARM;
	}

	uint16_t length = 0;
	swap_byte16((char *)&length, msgpack->read_buf_current);

	uint16_t limit = length;
	if (bufsize < length)
	{
		limit = bufsize;
	}

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	for (int i = 0; i < limit; i++)
	{
		if (!read_buf_shift_pos(msgpack, 1))
		{
			return ALARM;
		}
		*s++ = *msgpack->read_buf_current;
	}

	if (bufsize < length + 1)
	{
		printf("STR16 length error, string will be truncated\n");
		*--s = '\0';
	}
	else
	{
		*s = '\0';
	}

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	};

	return length;
}

static uint32_t str32_read_handler(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 3))
	{
		return ALARM;
	}
	uint32_t length = 0;
	swap_byte32((char *)&length, msgpack->read_buf_current);

	uint32_t limit = length;
	if (bufsize < length)
	{
		limit = bufsize;
	}

	if (!read_buf_shift_pos(msgpack, 3))
	{
		return ALARM;
	}

	for (int i = 0; i < limit; i++)
	{
		if (!read_buf_shift_pos(msgpack, 1))
		{
			return ALARM;
		}
		*s++ = *msgpack->read_buf_current;
	}

	if (bufsize < length + 1)
	{
		printf("STR32 length error, string will be truncated\n");
		*--s = '\0';
	}
	else
	{
		*s = '\0';
	}

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	};

	return length;
}

static uint8_t fixarray_read_handler(s_msgpack_t *msgpack)
{
	uint8_t size = (*msgpack->read_buf_current & FIXARRAY_NUMBER_MASK);

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return size;
}

static uint16_t array16_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	uint16_t size = 0;
	char *i_ptr = (char *)&size;

	swap_byte16(i_ptr, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, ARRAY16_LENGTH_SIZE))
	{
		return ALARM;
	}

	return size;
}

static uint32_t array32_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	uint32_t size = 0;
	char *i_ptr = (char *)&size;

	swap_byte32(i_ptr, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, ARRAY32_LENGTH_SIZE))
	{
		return ALARM;
	}

	return size;
}

static uint8_t fixmap_read_handler(s_msgpack_t *msgpack)
{
	uint8_t size = (*msgpack->read_buf_current & FIXMAP_NUMBER_MASK);

	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return size;
}

static uint16_t map16_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	uint16_t size = 0;
	char *i_ptr = (char *)&size;

	swap_byte16(i_ptr, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, MAP16_LENGTH_SIZE))
	{
		return ALARM;
	}

	return size;
}

static uint32_t map32_read_handler(s_msgpack_t *msgpack)
{
	if (!read_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	uint32_t size = 0;
	char *i_ptr = (char *)&size;

	swap_byte32(i_ptr, msgpack->read_buf_current);

	if (!read_buf_shift_pos(msgpack, MAP32_LENGTH_SIZE))
	{
		return ALARM;
	}

	return size;
}

static uint8_t true_read_handler(s_msgpack_t *msgpack)
{
	uint8_t response = OK;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

static uint8_t false_read_handler(s_msgpack_t *msgpack)
{
	uint8_t response = OK;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

static uint8_t nil_read_handler(s_msgpack_t *msgpack)
{
	uint8_t response = OK;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

static uint8_t neverused_read_handler(s_msgpack_t *msgpack)
{
	uint8_t response = OK;

	if (!read_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

/*****************************************************************************************/

/***************************************** READERS ***************************************/

uint8_t s_msgpack_int_read(s_msgpack_t *msgpack, int32_t *i)
{
	uint32_t r;
	uint8_t response = OK;
	if (s_msgpack_positive_fixint_read(msgpack, (uint8_t *)i))
	{
		r = *(uint8_t *)i;
	}
	else if (s_msgpack_negative_fixint_read(msgpack, (int8_t *)i))
	{
		r = *(int8_t *)i;
	}
	else if (s_msgpack_int8_read(msgpack, (int8_t *)i))
	{
		r = *(int8_t *)i;
	}
	else if (s_msgpack_uint8_read(msgpack, (uint8_t *)i))
	{
		r = *(uint8_t *)i;
	}
	else if (s_msgpack_int16_read(msgpack, (int16_t *)i))
	{
		r = *(int16_t *)i;
	}
	else if (s_msgpack_uint16_read(msgpack, (uint16_t *)i))
	{
		r = *(uint16_t *)i;
	}
	else if (s_msgpack_int32_read(msgpack, i))
	{
		r = *(int32_t *)i;
	}
	else if (s_msgpack_uint32_read(msgpack, i))
	{
		r = *(uint32_t *)i;
	}
	else
	{
		response = ALARM;
	}
	*i = r;
	return response;
}

uint8_t s_msgpack_uint_read(s_msgpack_t *msgpack, uint32_t *i)
{
	uint32_t r;
	uint8_t response = OK;
	if (s_msgpack_positive_fixint_read(msgpack, (uint8_t *)i))
	{
		r = *(uint8_t *)i;
	}
	else if (s_msgpack_uint8_read(msgpack, (uint8_t *)i))
	{
		r = *(uint8_t *)i;
	}
	else if (s_msgpack_uint16_read(msgpack, (uint16_t *)i))
	{
		r = *(uint16_t *)i;
	}
	else if (s_msgpack_uint32_read(msgpack, i))
	{
		r = *(uint32_t *)i;
	}
	else
	{
		response = ALARM;
	}
	*i = r;
	return response;
}

uint8_t s_msgpack_positive_fixint_read(s_msgpack_t *msgpack, uint8_t *ui)
{
	if (S_MSGPACK_TYPE_POSITIVE_FIXINT != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (positive_fixint_read_handler(msgpack))
	{
		*ui = msgpack->obj.ui8;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_negative_fixint_read(s_msgpack_t *msgpack, int8_t *i)
{
	if (S_MSGPACK_TYPE_NEGATIVE_FIXINT != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (negative_fixint_read_handler(msgpack))
	{
		*i = msgpack->obj.i8;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_int8_read(s_msgpack_t *msgpack, int8_t *i)
{
	if (S_MSGPACK_TYPE_INT8 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (int8_read_handler(msgpack))
	{
		*i = msgpack->obj.i8;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_uint8_read(s_msgpack_t *msgpack, uint8_t *ui)
{
	if (S_MSGPACK_TYPE_UINT8 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (uint8_read_handler(msgpack))
	{
		*ui = msgpack->obj.ui8;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_int16_read(s_msgpack_t *msgpack, int16_t *i)
{
	if (S_MSGPACK_TYPE_INT16 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (int16_read_handler(msgpack))
	{
		*i = msgpack->obj.i16;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_uint16_read(s_msgpack_t *msgpack, uint16_t *ui)
{
	if (S_MSGPACK_TYPE_UINT16 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (uint16_read_handler(msgpack))
	{
		*ui = msgpack->obj.ui16;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_int32_read(s_msgpack_t *msgpack, int32_t *i)
{
	if (S_MSGPACK_TYPE_INT32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	uint8_t response = ALARM;
	if (int32_read_handler(msgpack))
	{
		*i = msgpack->obj.i32;
		response = OK;
	}

	return response;
}

uint8_t s_msgpack_uint32_read(s_msgpack_t *msgpack, uint32_t *ui)
{
	if (S_MSGPACK_TYPE_UINT32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	if (uint32_read_handler(msgpack))
	{
		*ui = msgpack->obj.ui32;
	}
	return OK;
}

uint8_t s_msgpack_float_read(s_msgpack_t *msgpack, float *f)
{
	s_msgpack_float32_read(msgpack, f);
}

uint8_t s_msgpack_float32_read(s_msgpack_t *msgpack, float *f)
{
	if (S_MSGPACK_TYPE_FLOAT32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	if (float32_read_handler(msgpack))
	{
		*f = msgpack->obj.f;
	}
	return OK;
}

uint32_t s_msgpack_str_read(s_msgpack_t *msgpack, char *s, uint32_t size)
{
	if (size <= 0)
	{
		printf("str_read error, size must not be equal to 0");
		return 0;
	}

	return (s_msgpack_fixstr_read(msgpack, s, (uint8_t)size) ||
			  s_msgpack_str8_read(msgpack, s, (uint8_t)size) ||
			  s_msgpack_str16_read(msgpack, s, (uint16_t)size) ||
			  s_msgpack_str32_read(msgpack, s, size));
}

uint8_t s_msgpack_fixstr_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (S_MSGPACK_TYPE_FIXSTR != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return fixstr_read_handler(msgpack, s, bufsize);
}

uint8_t s_msgpack_str8_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (S_MSGPACK_TYPE_STR8 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return str8_read_handler(msgpack, s, bufsize);
}

uint16_t s_msgpack_str16_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (S_MSGPACK_TYPE_STR16 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return str16_read_handler(msgpack, s, bufsize);
}

uint32_t s_msgpack_str32_read(s_msgpack_t *msgpack, char *s, uint32_t bufsize)
{
	if (S_MSGPACK_TYPE_STR32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return str32_read_handler(msgpack, s, bufsize);
}

uint32_t s_msgpack_array_read(s_msgpack_t *msgpack)
{
	uint32_t result = 0;
	if (result = s_msgpack_fixarray_read(msgpack))
	{
		return result;
	}
	if (result = s_msgpack_array16_read(msgpack))
	{
		return result;
	}
	if (result = s_msgpack_array32_read(msgpack))
	{
		return result;
	}

	return result;
}

uint8_t s_msgpack_fixarray_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_FIXARRAY != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return fixarray_read_handler(msgpack);
}

uint16_t s_msgpack_array16_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_ARRAY16 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return array16_read_handler(msgpack);
}

uint32_t s_msgpack_array32_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_ARRAY32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return array32_read_handler(msgpack);
}

uint32_t s_msgpack_map_read(s_msgpack_t *msgpack)
{
	uint32_t result = 0;
	if (result = s_msgpack_fixmap_read(msgpack))
	{
		return result;
	}
	if (result = s_msgpack_map16_read(msgpack))
	{
		return result;
	}
	if (result = s_msgpack_map32_read(msgpack))
	{
		return result;
	}

	return result;
}

uint8_t s_msgpack_fixmap_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_FIXMAP != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return fixmap_read_handler(msgpack);
}

uint16_t s_msgpack_map16_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_MAP16 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return map16_read_handler(msgpack);
}

uint32_t s_msgpack_map32_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_MAP32 != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return map32_read_handler(msgpack);
}

uint8_t s_msgpack_true_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_TRUE != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return true_read_handler(msgpack);
}

uint8_t s_msgpack_false_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_FALSE != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return false_read_handler(msgpack);
}

uint8_t s_msgpack_bool_read(s_msgpack_t *msgpack, uint8_t *b)
{
	uint8_t response = OK;

	if (s_msgpack_true_read(msgpack))
	{
		*b = 1;
	}
	else if (s_msgpack_false_read(msgpack))
	{
		*b = 0;
	}
	else
	{
		response = ALARM;
	}

	return response;
}

uint8_t s_msgpack_nil_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_NIL != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return nil_read_handler(msgpack);
}

uint8_t s_msgpack_neverused_read(s_msgpack_t *msgpack)
{
	if (S_MSGPACK_TYPE_NEVERUSED != check_header(*msgpack->read_buf_current))
	{
		return ALARM;
	}

	return neverused_read_handler(msgpack);
}

uint8_t s_msgpack_read_elem(s_msgpack_t *msgpack, uint32_t elem_number)
{
	return read_elem_header_pos(msgpack, elem_number, 0);
}

/*****************************************************************************************/

/*************************************** WRITERS *****************************************/

uint8_t s_msgpack_uint_write(s_msgpack_t *msgpack, uint32_t ui)
{
	return (s_msgpack_positive_fixint_write(msgpack, ui) ||
			  s_msgpack_uint8_write(msgpack, ui) ||
			  s_msgpack_uint16_write(msgpack, ui) ||
			  s_msgpack_uint32_write(msgpack, ui));
}

uint8_t s_msgpack_int_write(s_msgpack_t *msgpack, int32_t ui)
{
	return (s_msgpack_negative_fixint_write(msgpack, ui) ||
			  s_msgpack_int8_write(msgpack, ui) ||
			  s_msgpack_int16_write(msgpack, ui) ||
			  s_msgpack_int32_write(msgpack, ui));
}

uint8_t s_msgpack_positive_fixint_write(s_msgpack_t *msgpack, uint8_t ui)
{
	if (ui < POSITIVE_FIXINT_NUMBER_MIN || ui > POSITIVE_FIXINT_NUMBER_MAX)
	{
		printf("positive_fixint_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = ui;

	uint8_t response = OK;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

uint8_t s_msgpack_negative_fixint_write(s_msgpack_t *msgpack, int8_t i)
{
	if (i > NEGATIVE_FIXINT_NUMBER_MAX || i < NEGATIVE_FIXINT_NUMBER_MIN)
	{
		printf("write_negative_fixint error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = i;

	uint8_t response = OK;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		response = ALARM;
	}

	return response;
}

uint8_t s_msgpack_int8_write(s_msgpack_t *msgpack, int8_t i)
{
	if (i > INT8_NUMBER_MAX || i < INT8_NUMBER_MIN)
	{
		printf("write_int8 error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = INT8_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = i;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_uint8_write(s_msgpack_t *msgpack, uint8_t ui)
{
	if (ui > UINT8_NUMBER_MAX || ui < UINT8_NUMBER_MIN)
	{
		printf("write_uint8 error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = UINT8_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = ui;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_int16_write(s_msgpack_t *msgpack, int16_t i)
{
	if (i > INT16_NUMBER_MAX || i < INT16_NUMBER_MIN)
	{
		printf("int16_write error, out of range\n");
		return ALARM;
	}

	if (write_buf_greather_limit(msgpack, INT16_DATA_SIZE))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = INT16_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *source = (char *)&i;
	char *destination = msgpack->write_buf_current;
	swap_byte16(destination, source);

	if (!write_buf_shift_pos(msgpack, INT16_DATA_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_uint16_write(s_msgpack_t *msgpack, uint16_t ui)
{
	if (ui > UINT16_NUMBER_MAX || ui < UINT16_NUMBER_MIN)
	{
		printf("uint16_write error, out of range\n");
		return ALARM;
	}

	if (write_buf_greather_limit(msgpack, UINT16_DATA_SIZE))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = UINT16_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *source = (char *)&ui;
	char *destination = msgpack->write_buf_current;
	swap_byte16(destination, source);

	if (!write_buf_shift_pos(msgpack, UINT16_DATA_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_int32_write(s_msgpack_t *msgpack, int32_t i)
{
	if (i > INT32_NUMBER_MAX || i < INT32_NUMBER_MIN)
	{
		printf("int32_write error, out of range\n");
		return ALARM;
	}

	if (write_buf_greather_limit(msgpack, INT32_DATA_SIZE))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = INT32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *source = (char *)&i;
	char *destination = msgpack->write_buf_current;
	swap_byte32(destination, source);

	if (!write_buf_shift_pos(msgpack, INT32_DATA_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_uint32_write(s_msgpack_t *msgpack, uint32_t ui)
{
	if (ui > UINT32_NUMBER_MAX || ui < UINT32_NUMBER_MIN)
	{
		printf("uint32_write error, out of range\n");
		return ALARM;
	}

	if (write_buf_greather_limit(msgpack, UINT32_DATA_SIZE))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = UINT32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *source = (char *)&ui;
	char *destination = msgpack->write_buf_current;
	swap_byte32(destination, source);

	if (!write_buf_shift_pos(msgpack, UINT32_DATA_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_float_write(s_msgpack_t *msgpack, float f)
{
	return s_msgpack_float32_write(msgpack, f);
}

uint8_t s_msgpack_float32_write(s_msgpack_t *msgpack, float f)
{
	if (f > FLOAT32_NUMBER_MAX || f < FLOAT32_NUMBER_MIN)
	{
		printf("float32_write error, out of range\n");
		return ALARM;
	}

	if (write_buf_greather_limit(msgpack, FLOAT32_DATA_SIZE))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = FLOAT32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *source = (char *)&f;
	char *destination = msgpack->write_buf_current;
	swap_byte32(destination, source);

	if (!write_buf_shift_pos(msgpack, FLOAT32_DATA_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_str_write(s_msgpack_t *msgpack, char *s, uint32_t size)
{
	return (s_msgpack_fixstr_write(msgpack, s, size) ||
			  s_msgpack_str8_write(msgpack, s, size) ||
			  s_msgpack_str16_write(msgpack, s, size) ||
			  s_msgpack_str32_write(msgpack, s, size));
}

uint8_t s_msgpack_fixstr_write(s_msgpack_t *msgpack, char *s, uint8_t size)
{
	if (write_buf_greather_limit(msgpack, size))
	{
		return ALARM;
	}
	else if (size > FIXSTR_NUMBER_MAX)
	{
		printf("fixstr_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = FIXSTR_ID | size;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *destination = msgpack->write_buf_current;
	char *source = s;
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
	}

	if (!write_buf_shift_pos(msgpack, size))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_str8_write(s_msgpack_t *msgpack, char *s, uint8_t size)
{
	/* потому что размер */
	if (write_buf_greather_limit(msgpack, size + 1))
	{
		return ALARM;
	}
	else if (size > STR8_NUMBER_MAX)
	{
		printf("str8_write error, out of range\n");
		return 0;
	}

	*msgpack->write_buf_current = STR8_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	*msgpack->write_buf_current = size;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *destination = msgpack->write_buf_current;
	char *source = s;
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
	}

	if (!write_buf_shift_pos(msgpack, size))
	{
		return ALARM;
	}

	return OK;
}

uint16_t s_msgpack_str16_write(s_msgpack_t *msgpack, char *s, uint16_t size)
{
	/* потому что поле размер */
	if (write_buf_greather_limit(msgpack, size + 2))
	{
		return ALARM;
	}
	else if (size > STR16_NUMBER_MAX)
	{
		printf("str16_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = STR16_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte16(dst, src);

	if (!write_buf_shift_pos(msgpack, STR16_LENGTH_SIZE))
	{
		return ALARM;
	}

	char *destination = msgpack->write_buf_current;
	char *source = s;
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
	}

	if (!write_buf_shift_pos(msgpack, size))
	{
		return ALARM;
	}

	return OK;
}

uint32_t s_msgpack_str32_write(s_msgpack_t *msgpack, char *s, uint32_t size)
{
	/* потому что поле размера */
	if (write_buf_greather_limit(msgpack, size + 4))
	{
		return ALARM;
	}
	else if (size > STR32_NUMBER_MAX)
	{
		printf("str32_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = STR32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte32(dst, src);

	if (!write_buf_shift_pos(msgpack, STR32_LENGTH_SIZE))
	{
		return ALARM;
	}

	char *destination = msgpack->write_buf_current;
	char *source = s;
	for (int i = 0; i < size; i++)
	{
		destination[i] = source[i];
	}

	if (!write_buf_shift_pos(msgpack, size))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_array_write(s_msgpack_t *msgpack, uint32_t size)
{
	return (s_msgpack_fixarray_write(msgpack, size) ||
			  s_msgpack_array16_write(msgpack, size) ||
			  s_msgpack_array32_write(msgpack, size));
}

uint8_t s_msgpack_fixarray_write(s_msgpack_t *msgpack, uint8_t size)
{
	if (write_buf_greather_limit(msgpack, size))
	{
		return ALARM;
	}
	else if (size > FIXARRAY_NUMBER_MAX)
	{
		printf("fixarray_write error, bufsize overflow\n");
		return ALARM;
	}

	*msgpack->write_buf_current = FIXARRAY_ID | size;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_array16_write(s_msgpack_t *msgpack, uint16_t size)
{
	/* потому что поле размера */
	if (write_buf_greather_limit(msgpack, size + 2))
	{
		return ALARM;
	}
	else if (size > ARRAY16_NUMBER_MAX)
	{
		printf("array16_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = ARRAY16_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte16(dst, src);

	if (!write_buf_shift_pos(msgpack, ARRAY16_LENGTH_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_array32_write(s_msgpack_t *msgpack, uint32_t size)
{
	/* потому что поле размера */
	if (write_buf_greather_limit(msgpack, size + 4))
	{
		return ALARM;
	}
	else if (size > ARRAY32_NUMBER_MAX)
	{
		printf("array32_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = ARRAY32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte32(dst, src);

	if (!write_buf_shift_pos(msgpack, ARRAY32_LENGTH_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_map_write(s_msgpack_t *msgpack, uint32_t size)
{
	return (s_msgpack_fixmap_write(msgpack, size) ||
			  s_msgpack_map16_write(msgpack, size) ||
			  s_msgpack_map32_write(msgpack, size));
}

uint8_t s_msgpack_fixmap_write(s_msgpack_t *msgpack, uint8_t size)
{
	if (write_buf_greather_limit(msgpack, size * 2))
	{
		return ALARM;
	}
	else if (size > FIXMAP_NUMBER_MAX)
	{
		printf("fixmap_write error, bufsize overflow\n");
		return ALARM;
	}

	*msgpack->write_buf_current = FIXMAP_ID | size;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_map16_write(s_msgpack_t *msgpack, uint16_t size)
{
	/* потому что поле размера */
	if (write_buf_greather_limit(msgpack, (size * 2) + 2))
	{
		return ALARM;
	}
	else if (size > MAP16_NUMBER_MAX)
	{
		printf("map16_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = MAP16_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte16(dst, src);

	if (!write_buf_shift_pos(msgpack, MAP16_LENGTH_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_map32_write(s_msgpack_t *msgpack, uint32_t size)
{
	/* потому что поле размера */
	if (write_buf_greather_limit(msgpack, (size * 2) + 4))
	{
		return ALARM;
	}
	else if (size > MAP32_NUMBER_MAX)
	{
		printf("map32_write error, out of range\n");
		return ALARM;
	}

	*msgpack->write_buf_current = MAP32_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}

	char *src = (char *)&size;
	char *dst = msgpack->write_buf_current;
	swap_byte32(dst, src);

	if (!write_buf_shift_pos(msgpack, MAP32_LENGTH_SIZE))
	{
		return ALARM;
	}

	return OK;
}

uint8_t s_msgpack_bool_write(s_msgpack_t *msgpack, uint8_t bool_var)
{
	uint8_t response;
	if (bool_var)
	{
		response = s_msgpack_true_write(msgpack);
	}
	else
	{
		response = s_msgpack_false_write(msgpack);
	}

	return response;
}

uint8_t s_msgpack_true_write(s_msgpack_t *msgpack)
{
	*msgpack->write_buf_current = TRUE_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}
	return OK;
}

uint8_t s_msgpack_false_write(s_msgpack_t *msgpack)
{
	*msgpack->write_buf_current = FALSE_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}
	return OK;
}

uint8_t s_msgpack_nil_write(s_msgpack_t *msgpack)
{
	*msgpack->write_buf_current = NIL_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}
	return OK;
}

uint8_t s_msgpack_neverused_write(s_msgpack_t *msgpack)
{
	*msgpack->write_buf_current = NEVER_USED_ID;
	if (!write_buf_shift_pos(msgpack, 1))
	{
		return ALARM;
	}
	return OK;
}

/*****************************************************************************************/

/********************************** OPTIONAL *********************************************/

static void swap_byte16(char *number_ptr, char *msgpack)
{
	number_ptr[0] = msgpack[1];
	number_ptr[1] = msgpack[0];
}

static void swap_byte32(char *number_ptr, char *msgpack)
{
	number_ptr[0] = msgpack[3];
	number_ptr[1] = msgpack[2];
	number_ptr[2] = msgpack[1];
	number_ptr[3] = msgpack[0];
}

static void swap_byte64(char *number_ptr, char *msgpack)
{
	number_ptr[0] = msgpack[7];
	number_ptr[1] = msgpack[6];
	number_ptr[2] = msgpack[5];
	number_ptr[3] = msgpack[4];
	number_ptr[4] = msgpack[3];
	number_ptr[5] = msgpack[2];
	number_ptr[6] = msgpack[1];
	number_ptr[7] = msgpack[0];
}

/*****************************************************************************************/

/*************************************** READ BUFFERS ************************************/

static uint8_t read_buf_shift_pos(s_msgpack_t *msgpack, uint32_t shift)
{
	uint8_t response;
	if (msgpack->read_buf_pos + shift <= msgpack->read_buf_size - 1)
	{
		msgpack->read_buf_current = msgpack->read_buf_current + shift;
		msgpack->read_buf_pos = msgpack->read_buf_pos + shift;
		response = OK;
	}
	else
	{
		printf("Error bufsize an read buffer\n");
		response = ALARM;
	}
	return response;
}

static uint8_t read_buf_greather_limit(s_msgpack_t *msgpack, uint32_t shift)
{
	return ((msgpack->read_buf_pos + shift) > msgpack->read_buf_size - 1);
}

void s_msgpack_set_read_buf(s_msgpack_t *s_msgpack, char *msgpack, uint32_t bufsize)
{
	s_msgpack->read_buf_start = msgpack;
	s_msgpack->read_buf_current = msgpack;
	s_msgpack->read_buf_size = bufsize;
	s_msgpack->read_buf_pos = 0;
}

void s_msgpack_set_to_start_read_buf(s_msgpack_t *s_msgpack)
{
	s_msgpack->read_buf_pos = 0;
	s_msgpack->read_buf_current = s_msgpack->read_buf_start;
}

uint8_t s_msgpack_set_to_pos_read_buf(s_msgpack_t *s_msgpack, uint32_t pos)
{
	if (s_msgpack->read_buf_size - 1 < pos)
	{
		printf("set_to_pos_read_buf error, pos greather then read_buf_size\n");
		return ALARM;
	}

	s_msgpack->read_buf_pos = pos;
	s_msgpack->read_buf_current = s_msgpack->read_buf_start + pos;

	return OK;
}

uint8_t s_msgpack_set_read_buf_pos_to_elem(s_msgpack_t *msgpack, uint32_t elem)
{
	uint32_t elem_pos = read_elem_header_pos(msgpack, elem, 1);
	return s_msgpack_set_to_pos_read_buf(msgpack, elem_pos);
}

uint32_t s_msgpack_get_read_buf_pos(s_msgpack_t *s_msgpack)
{
	return s_msgpack->read_buf_pos;
}

uint8_t s_msgpack_get_read_buf_header(s_msgpack_t *s_msgpack)
{
	return check_header(*s_msgpack->read_buf_current);
}

uint32_t s_msgpack_get_read_buf_pos_elem(s_msgpack_t *msgpack, uint32_t elem)
{
	return read_elem_header_pos(msgpack, elem, 1);
}

uint32_t s_msgpack_get_read_buf_length(s_msgpack_t *msgpack)
{
	return msgpack->read_buf_size;
}

char *s_msgpack_get_read_buf(s_msgpack_t *s_msgpack)
{
	return s_msgpack->read_buf_start;
}

unsigned char s_msgpack_get_read_buf_byte(s_msgpack_t *msgpack)
{
	return *msgpack->read_buf_current;
}

unsigned char s_msgpack_get_read_buf_pos_byte(s_msgpack_t *msgpack, uint32_t pos)
{
	return *(msgpack->read_buf_start + pos);
}

/*****************************************************************************************/

/************************************** WRITE BUFFERS ************************************/

static uint8_t write_buf_shift_pos(s_msgpack_t *msgpack, uint32_t shift)
{
	uint8_t response;
	if (msgpack->write_buf_pos + shift <= msgpack->write_buf_size - 1)
	{
		msgpack->write_buf_current = msgpack->write_buf_current + shift;
		msgpack->write_buf_pos = msgpack->write_buf_pos + shift;
		response = OK;
	}
	else
	{
		printf("error bufsize an write buffer\n");
		response = ALARM;
	}
	return response;
}

static uint8_t write_buf_greather_limit(s_msgpack_t *msgpack, uint32_t shift)
{
	return ((msgpack->write_buf_pos + shift) > msgpack->write_buf_size - 1);
}

void s_msgpack_set_write_buf(s_msgpack_t *s_msgpack, char *msgpack, uint32_t bufsize)
{
	s_msgpack->write_buf_start = msgpack;
	s_msgpack->write_buf_current = msgpack;
	s_msgpack->write_buf_size = bufsize;
	s_msgpack->write_buf_pos = 0;
}

void s_msgpack_set_to_start_write_buf(s_msgpack_t *s_msgpack)
{
	s_msgpack->write_buf_pos = 0;
	s_msgpack->write_buf_current = s_msgpack->write_buf_start;
}

uint8_t s_msgpack_set_to_pos_write_buf(s_msgpack_t *s_msgpack, uint32_t pos)
{
	if (s_msgpack->write_buf_size - 1 < pos)
	{
		printf("set_to_pos_write_buf error, pos greather then write_buf_size\n");
		return ALARM;
	}

	s_msgpack->write_buf_pos = pos;
	s_msgpack->write_buf_current = s_msgpack->write_buf_start + pos;

	return OK;
}

uint32_t s_msgpack_get_write_buf_pos(s_msgpack_t *s_msgpack)
{
	return s_msgpack->write_buf_pos;
}

uint8_t s_msgpack_get_write_buf_header(s_msgpack_t *s_msgpack)
{
	return check_header(*s_msgpack->write_buf_current);
}

uint32_t s_msgpack_get_write_buf_length(s_msgpack_t *msgpack)
{
	return msgpack->write_buf_size;
}

char *s_msgpack_get_write_buf(s_msgpack_t *s_msgpack)
{
	return s_msgpack->write_buf_start;
}

unsigned char s_msgpack_get_write_buf_byte(s_msgpack_t *msgpack)
{
	return *msgpack->write_buf_current;
}

unsigned char s_msgpack_get_write_buf_pos_byte(s_msgpack_t *msgpack, uint32_t pos)
{
	return *(msgpack->write_buf_start + pos);
}

/*****************************************************************************************/

/********************************* CHECKS_TYPE *******************************************/

uint8_t s_msgpack_is_array(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_FIXARRAY ||
			  header == S_MSGPACK_TYPE_ARRAY16 ||
			  header == S_MSGPACK_TYPE_ARRAY32);
}

uint8_t s_msgpack_is_str(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_FIXSTR ||
			  header == S_MSGPACK_TYPE_STR8 ||
			  header == S_MSGPACK_TYPE_STR16 ||
			  header == S_MSGPACK_TYPE_STR32);
}

uint8_t s_msgpack_is_bool(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_TRUE ||
			  header == S_MSGPACK_TYPE_FALSE);
}

uint8_t s_msgpack_is_numeric(uint8_t header)
{
	return (s_msgpack_is_int(header) || s_msgpack_is_uint(header) || s_msgpack_is_float(header));
}

uint8_t s_msgpack_is_int(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_NEGATIVE_FIXINT ||
			  header == S_MSGPACK_TYPE_INT8 ||
			  header == S_MSGPACK_TYPE_INT16 ||
			  header == S_MSGPACK_TYPE_INT32);
}

uint8_t s_msgpack_is_uint(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_POSITIVE_FIXINT ||
			  header == S_MSGPACK_TYPE_UINT8 ||
			  header == S_MSGPACK_TYPE_UINT16 ||
			  header == S_MSGPACK_TYPE_UINT32);
}

uint8_t s_msgpack_is_float(uint8_t header)
{
	return (header == S_MSGPACK_TYPE_FLOAT32);
}

/*****************************************************************************************/

uint8_t s_msgpack_read_to_str(s_msgpack_t *s_msgpack, char *str, const uint64_t str_size)
{
	uint64_t cur_size = 0;
	return (start_read_to_str(s_msgpack, str, str_size, &cur_size) && cpy_msg_to_str(str, str_size, &cur_size, "\0", sizeof("\0")));
}

uint8_t s_msgpack_write_from_str()
{
}

static uint8_t start_read_to_str(s_msgpack_t *s_msgpack, char *str, const uint64_t str_size, uint64_t *cur_size)
{
	uint8_t response = OK;
	uint8_t header = check_header(*s_msgpack->read_buf_current);

	switch (header)
	{
	case S_MSGPACK_TYPE_POSITIVE_FIXINT:
		if (!positive_fixint_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 4;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%d", s_msgpack->obj.ui8);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_NEGATIVE_FIXINT:
		if (!negative_fixint_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}

		{
			const uint8_t temp_max_size = 5;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%d", s_msgpack->obj.i8);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_UINT8:
		if (!uint8_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 4;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%u", s_msgpack->obj.ui8);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_UINT16:
		if (!uint16_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 6;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%u", s_msgpack->obj.ui16);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_UINT32:
		if (!uint32_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 11;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%u", s_msgpack->obj.ui32);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_INT8:
		if (!int8_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 5;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%d", s_msgpack->obj.i8);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_INT16:
		if (!int16_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 7;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%d", s_msgpack->obj.i16);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_INT32:
		if (!int32_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 12;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%d", s_msgpack->obj.i32);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_FLOAT32:
		if (!float32_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			const uint8_t temp_max_size = 50;
			char temp[temp_max_size];
			if (str_size < *cur_size + temp_max_size)
			{
				printf("read_to_str error, oversize\n");
				response = ALARM;
				break;
			}
			uint8_t r_size = snprintf(temp, temp_max_size, "%.3f", s_msgpack->obj.f);
			strncpy(str + *cur_size, temp, r_size);

			*cur_size += r_size;
		}
		break;

	case S_MSGPACK_TYPE_FIXSTR:
	{
		char q_msg[] = "\"";
		uint8_t q_msg_size = sizeof(q_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}

		uint8_t r_size = fixstr_read_handler(s_msgpack, str + *cur_size, str_size - *cur_size);
		if (!r_size)
		{
			response = ALARM;
			break;
		}
		*cur_size += r_size;

		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_STR8:
	{
		char q_msg[] = "\"";
		uint8_t q_msg_size = sizeof(q_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}

		uint8_t r_size = str8_read_handler(s_msgpack, str + *cur_size, str_size - *cur_size);
		if (!r_size)
		{
			response = ALARM;
			break;
		}
		*cur_size += r_size;

		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_STR16:
	{
		char q_msg[] = "\"";
		uint8_t q_msg_size = sizeof(q_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}

		uint16_t r_size = str16_read_handler(s_msgpack, str + *cur_size, str_size - *cur_size);
		if (!r_size)
		{
			response = ALARM;
			break;
		}
		*cur_size += r_size;

		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_STR32:
	{
		char q_msg[] = "\"";
		uint8_t q_msg_size = sizeof(q_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}

		uint32_t r_size = str32_read_handler(s_msgpack, str + *cur_size, str_size - *cur_size);
		if (!r_size)
		{
			response = ALARM;
			break;
		}
		*cur_size += r_size;

		if (!cpy_msg_to_str(str, str_size, cur_size, q_msg, q_msg_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_FIXARRAY:
	{
		uint8_t ar_size = (*s_msgpack->read_buf_current & FIXARRAY_NUMBER_MASK);
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		if (!process_read_array(s_msgpack, ar_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_ARRAY16:
	{
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		uint16_t ar_size = 0;
		char *i_ptr = (char *)&ar_size;

		swap_byte16(i_ptr, s_msgpack->read_buf_current);

		if (!read_buf_shift_pos(s_msgpack, 2))
		{
			response = ALARM;
			break;
		}

		if (!process_read_array(s_msgpack, ar_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_ARRAY32:
	{
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		uint32_t ar_size = 0;
		char *i_ptr = (char *)&ar_size;

		swap_byte32(i_ptr, s_msgpack->read_buf_current);

		if (!read_buf_shift_pos(s_msgpack, 4))
		{
			response = ALARM;
			break;
		}

		if (!process_read_array(s_msgpack, ar_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_FIXMAP:
	{
		uint8_t map_size = *s_msgpack->read_buf_current & FIXARRAY_NUMBER_MASK;
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		if (!process_read_map(s_msgpack, map_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_MAP16:
	{
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		uint16_t map_size = 0;
		char *i_ptr = (char *)&map_size;

		swap_byte16(i_ptr, s_msgpack->read_buf_current);

		if (!read_buf_shift_pos(s_msgpack, 2))
		{
			response = ALARM;
			break;
		}

		if (!process_read_map(s_msgpack, map_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_MAP32:
	{
		if (!read_buf_shift_pos(s_msgpack, 1))
		{
			response = ALARM;
			break;
		}

		uint32_t map_size = 0;
		char *i_ptr = (char *)&map_size;

		swap_byte32(i_ptr, s_msgpack->read_buf_current);

		if (!read_buf_shift_pos(s_msgpack, 4))
		{
			response = ALARM;
			break;
		}

		if (!process_read_map(s_msgpack, map_size, str, str_size, cur_size))
		{
			response = ALARM;
			break;
		}
	}
	break;

	case S_MSGPACK_TYPE_TRUE:
	{
		if (!true_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			char true_msg[] = "true";
			uint8_t true_msg_size = sizeof(true_msg) - 1;

			if (!cpy_msg_to_str(str, str_size, cur_size, true_msg, true_msg_size))
			{
				response = ALARM;
				break;
			}
		}
	}
	break;

	case S_MSGPACK_TYPE_FALSE:
	{
		if (!false_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			char false_msg[] = "false";
			uint8_t false_msg_size = sizeof(false_msg) - 1;

			if (!cpy_msg_to_str(str, str_size, cur_size, false_msg, false_msg_size))
			{
				response = ALARM;
				break;
			}
		}
	}
	break;

	case S_MSGPACK_TYPE_NEVERUSED:
	{
		if (!neverused_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			char neverused_msg[] = "(neverused)";
			uint8_t neverused_msg_size = sizeof(neverused_msg) - 1;

			if (!cpy_msg_to_str(str, str_size, cur_size, neverused_msg, neverused_msg_size))
			{
				response = ALARM;
				break;
			}
		}
	}
	break;

	case S_MSGPACK_TYPE_NIL:
	{
		if (!nil_read_handler(s_msgpack))
		{
			response = ALARM;
			break;
		}
		{
			char nil_msg[] = "null";
			uint8_t nil_msg_size = sizeof(nil_msg) - 1;

			if (!cpy_msg_to_str(str, str_size, cur_size, nil_msg, nil_msg_size))
			{
				response = ALARM;
				break;
			}
		}
	}
	break;

	default:
		strncpy(str, "UNDEFINED_HEADER", 17);
	}

	return response;
}

static uint8_t cpy_msg_to_str(char *str, uint64_t str_size, uint64_t *cur_size, char *msg, uint32_t msg_size)
{
	if (*cur_size + msg_size > str_size)
	{
		printf("read_to_str error, oversize\n");
		return ALARM;
	}

	strncpy(str + *cur_size, msg, msg_size);
	*cur_size += msg_size;

	return OK;
}

static uint8_t process_read_map(s_msgpack_t *s_msgpack, uint32_t map_size, char *str, uint64_t str_size, uint64_t *cur_size)
{
	if (map_size == 0)
	{
		char p_msg[] = "{}";
		uint8_t p_msg_size = sizeof(p_msg) - 1;

		if (!cpy_msg_to_str(str, str_size, cur_size, p_msg, p_msg_size))
		{
			return ALARM;
		}
	}
	else
	{
		char start_msg[] = "{";
		uint8_t start_msg_size = sizeof(start_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, start_msg, start_msg_size))
		{
			return ALARM;
		}

		for (int i = 0; i < map_size; i++)
		{
			start_read_to_str(s_msgpack, str, str_size, cur_size);

			char split_msg[] = ":";
			uint8_t split_msg_size = sizeof(split_msg) - 1;

			if (!cpy_msg_to_str(str, str_size, cur_size, split_msg, split_msg_size))
			{
				return ALARM;
			}

			start_read_to_str(s_msgpack, str, str_size, cur_size);

			if (i < map_size - 1)
			{
				char mid_msg[] = ",";
				uint8_t mid_msg_size = sizeof(mid_msg) - 1;

				if (!cpy_msg_to_str(str, str_size, cur_size, mid_msg, mid_msg_size))
				{
					return ALARM;
				}
			}
		}

		char end_msg[] = "}";
		uint8_t end_msg_size = sizeof(end_msg) - 1;

		if (!cpy_msg_to_str(str, str_size, cur_size, end_msg, end_msg_size))
		{
			return ALARM;
		}
	}
	return OK;
}

static uint8_t process_read_array(s_msgpack_t *s_msgpack, uint32_t ar_size, char *str, uint64_t str_size, uint64_t *cur_size)
{
	if (ar_size == 0)
	{
		char p_msg[] = "[]";
		uint8_t p_msg_size = sizeof(p_msg) - 1;

		if (!cpy_msg_to_str(str, str_size, cur_size, p_msg, p_msg_size))
		{
			return ALARM;
		}
	}
	else
	{
		char start_msg[] = "[";
		uint8_t start_msg_size = sizeof(start_msg) - 1;
		if (!cpy_msg_to_str(str, str_size, cur_size, start_msg, start_msg_size))
		{
			return ALARM;
		}

		for (int i = 0; i < ar_size; i++)
		{
			start_read_to_str(s_msgpack, str, str_size, cur_size);
			if (i < ar_size - 1)
			{
				char mid_msg[] = ",";
				uint8_t mid_msg_size = sizeof(mid_msg) - 1;

				if (!cpy_msg_to_str(str, str_size, cur_size, mid_msg, mid_msg_size))
				{
					return ALARM;
				}
			}
		}

		char end_msg[] = "]";
		uint8_t end_msg_size = sizeof(end_msg) - 1;

		if (!cpy_msg_to_str(str, str_size, cur_size, end_msg, end_msg_size))
		{
			return ALARM;
		}
	}
	return OK;
}

static uint32_t read_elem_header_pos(s_msgpack_t *msgpack, uint32_t elem_number, uint8_t pos_mode)
{
	if (elem_number == 0)
	{
		return ALARM;
	}

	uint32_t save_pos = msgpack->read_buf_pos;
	s_msgpack_set_to_start_read_buf(msgpack);
	uint32_t header = S_MSGPACK_TYPE_EMPTY;
	uint32_t response = OK;

	if (pos_mode)
	{
		if (elem_number == 1)
		{
			return msgpack->read_buf_pos;
		}
		elem_number = elem_number - 1;
	}
	else
	{
		if (elem_number == 1)
		{
			return s_msgpack_get_read_buf_header(msgpack);
		}
	}

	for (int i = 0; i < elem_number; i++)
	{
		header = check_header(*msgpack->read_buf_current);
		switch (header)
		{
		case S_MSGPACK_TYPE_TRUE:
		case S_MSGPACK_TYPE_FALSE:
		case S_MSGPACK_TYPE_NIL:
		case S_MSGPACK_TYPE_NEVERUSED:
		case S_MSGPACK_TYPE_POSITIVE_FIXINT:
		case S_MSGPACK_TYPE_NEGATIVE_FIXINT:
		case S_MSGPACK_TYPE_FIXARRAY:
		case S_MSGPACK_TYPE_FIXMAP:
			if (!read_buf_shift_pos(msgpack, 1))
			{
				response = ALARM;
			}
			break;

		case S_MSGPACK_TYPE_UINT8:
		case S_MSGPACK_TYPE_INT8:
			if (!read_buf_shift_pos(msgpack, 2))
			{
				response = ALARM;
			}
			break;

		case S_MSGPACK_TYPE_UINT16:
		case S_MSGPACK_TYPE_INT16:
		case S_MSGPACK_TYPE_ARRAY16:
		case S_MSGPACK_TYPE_MAP16:
			if (!read_buf_shift_pos(msgpack, 3))
			{
				response = ALARM;
			}
			break;

		case S_MSGPACK_TYPE_UINT32:
		case S_MSGPACK_TYPE_INT32:
		case S_MSGPACK_TYPE_FLOAT32:
		case S_MSGPACK_TYPE_ARRAY32:
		case S_MSGPACK_TYPE_MAP32:
			if (!read_buf_shift_pos(msgpack, 5))
			{
				response = ALARM;
			}
			break;

		case S_MSGPACK_TYPE_FIXSTR:
		{
			uint8_t length = (*msgpack->read_buf_current & FIXSTR_NUMBER_MASK);

			if (!read_buf_shift_pos(msgpack, length + 1))
			{
				response = ALARM;
			}
			break;
		}

		case S_MSGPACK_TYPE_STR8:
		{
			if (!read_buf_shift_pos(msgpack, 1))
			{
				response = ALARM;
			}

			uint8_t length = *msgpack->read_buf_current;
			if (!read_buf_shift_pos(msgpack, length + 1))
			{
				response = ALARM;
			}
			break;
		}

		case S_MSGPACK_TYPE_STR16:
		{
			if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 1))
			{
				response = ALARM;
			}

			uint16_t length = 0;
			swap_byte16((char *)&length, msgpack->read_buf_current);
			if (!read_buf_shift_pos(msgpack, length + 1))
			{
				response = ALARM;
			}
			break;
		}

		case S_MSGPACK_TYPE_STR32:
		{
			if (!read_buf_shift_pos(msgpack, 1) || read_buf_greather_limit(msgpack, 3))
			{
				response = ALARM;
			}
			uint32_t length = 0;
			swap_byte32((char *)&length, msgpack->read_buf_current);

			if (!read_buf_shift_pos(msgpack, length + 1))
			{
				response = ALARM;
			}
			break;
		}
		}

		if (header == S_MSGPACK_TYPE_EMPTY || response == ALARM)
		{
			break;
		}
	}

	if (header != S_MSGPACK_TYPE_EMPTY || response != ALARM)
	{
		if (pos_mode)
		{
			response = msgpack->read_buf_pos;
		}
		else
		{
			response = header;
		}
	}
	s_msgpack_set_to_pos_read_buf(msgpack, save_pos);
	return response;
}

uint8_t s_msgpack_type_to_str(uint8_t header, char *str, uint8_t str_size)
{
	uint8_t min_length = 17;
	if (str_size < min_length)
	{
		return ALARM;
	}

	switch (header)
	{
	case S_MSGPACK_TYPE_EMPTY:
		strncpy(str, "EMPTY", 6);
		break;
	case S_MSGPACK_TYPE_POSITIVE_FIXINT:
		strncpy(str, "POSITIVE_FIXINT", 16);
		break;
	case S_MSGPACK_TYPE_NEGATIVE_FIXINT:
		strncpy(str, "NEGATIVE_FIXINT", 16);
		break;
	case S_MSGPACK_TYPE_UINT8:
		strncpy(str, "UINT8", 6);
		break;
	case S_MSGPACK_TYPE_UINT16:
		strncpy(str, "UINT16", 7);
		break;
	case S_MSGPACK_TYPE_UINT32:
		strncpy(str, "UINT32", 7);
		break;
	case S_MSGPACK_TYPE_INT8:
		strncpy(str, "INT8", 5);
		break;
	case S_MSGPACK_TYPE_INT16:
		strncpy(str, "INT16", 6);
		break;
	case S_MSGPACK_TYPE_INT32:
		strncpy(str, "INT32", 6);
		break;
	case S_MSGPACK_TYPE_FLOAT32:
		strncpy(str, "FLOAT32", 8);
		break;
	case S_MSGPACK_TYPE_FIXSTR:
		strncpy(str, "FIXSTR", 7);
		break;
	case S_MSGPACK_TYPE_STR8:
		strncpy(str, "STR8", 5);
		break;
	case S_MSGPACK_TYPE_STR16:
		strncpy(str, "STR16", 6);
		break;
	case S_MSGPACK_TYPE_STR32:
		strncpy(str, "STR32", 6);
		break;
	case S_MSGPACK_TYPE_FIXARRAY:
		strncpy(str, "FIXARRAY", 9);
		break;
	case S_MSGPACK_TYPE_ARRAY16:
		strncpy(str, "ARRAY16", 8);
		break;
	case S_MSGPACK_TYPE_ARRAY32:
		strncpy(str, "ARRAY32", 8);
		break;
	case S_MSGPACK_TYPE_FIXMAP:
		strncpy(str, "FIXMAP", 7);
		break;
	case S_MSGPACK_TYPE_MAP16:
		strncpy(str, "MAP16", 6);
		break;
	case S_MSGPACK_TYPE_MAP32:
		strncpy(str, "MAP32", 6);
		break;
	case S_MSGPACK_TYPE_TRUE:
		strncpy(str, "TRUE", 5);
		break;
	case S_MSGPACK_TYPE_FALSE:
		strncpy(str, "FALSE", 6);
		break;
	case S_MSGPACK_TYPE_NEVERUSED:
		strncpy(str, "NEVERUSED", 10);
		break;
	case S_MSGPACK_TYPE_NIL:
		strncpy(str, "NIL", 4);
		break;
	default:
		strncpy(str, "UNDEFINED_HEADER", 17);
	}

	return OK;
}