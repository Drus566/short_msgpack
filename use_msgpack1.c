#include "./lib/short_msgpack/s_msgpack.h"

/*
	НЕ РЕАЛИЗОВАНО:
	bin 8
	bin 16
	bin 32
	ext 8
	ext 16
	ext 32
	float 64
	uint 64
	int 64
	fixext 1
	fixext 2
	fixext 4
	fixext 8
	fixext 16
*/

int main() 
{
	/*** Примеры с чтением msgpack сообщений */

	int BUFSIZE = 1024;
	/* Создаем структурку */
	s_msgpack_t s_msgpack;
	/* Два буфера для ввода и вывода под структурку */
	char buffer_read[BUFSIZE];
	char buffer_write[BUFSIZE];

	/* Инициализируем, кладем структурку,
	 буфер откуда будем читать,
	 его размер,
	 буфер куда будем писать,
	 его размер
	*/
	s_msgpack_init(&s_msgpack, 
	                buffer_read, 
					BUFSIZE, 
					buffer_write,
					BUFSIZE);

	// Get the goodnes read

	/* Заводим строчку для удобного вывода данных */
	char str_for_print[BUFSIZE];

	/* Пример считывания данных int8 */
	char data1[] = { 0xd0, 0x81 };
	
	/* Меняем буфер для чтения на новый...
	                Заметка 
	Всегда нужно указывать размер буфера на 1 больше,
	чем есть там данных на самом деле (ЭТО ВАЖНО, иначе будет ошибка) 
	*/
	s_msgpack_set_read_buf(&s_msgpack, data1, 3);

	/* Считываем в строчку и выводим её, 
	возвращает результат чтения 1 - успешно 0 - нет 
	*/
	int result = s_msgpack_read_to_str(&s_msgpack, str_for_print, BUFSIZE);
	printf("Result: %d, str: %s\n", result, str_for_print);

	/* Ставим указатель в начало буфера чтения */
	s_msgpack_set_to_start_read_buf(&s_msgpack);

	/* Считываем в переменную */
	int8_t i_var = 0;
	s_msgpack_int8_read(&s_msgpack, &i_var);
	printf("int8: %d\n", i_var);

	/* Все попытки чтения возвращают либо количество считанных байт (значит чтение прошло успешно),
	 либо размер контейнера (значит чтение прошло успешно), 
	 либо успешно - 1 или неуспешно - 0 (в случае если длина считываемого типа не равна 0) прошло считывание 
	*/
	/* Пробуем читать uint8 */
	char data2[] = { 0xcc, 0x8f };
	s_msgpack_set_read_buf(&s_msgpack, data2, 3);

	uint8_t ui_var = 0;
	s_msgpack_uint8_read(&s_msgpack, &ui_var);
	printf("uint8: %u\n", ui_var);

	/* positive fixint */
	char data3[] = { 0x70 };
	s_msgpack_set_read_buf(&s_msgpack, data3, 2);

	s_msgpack_positive_fixint_read(&s_msgpack, &ui_var);
	printf("positive fixint: %u\n", ui_var);

	/* negative fixint */
	char data4[] = { 0xe2 };
	s_msgpack_set_read_buf(&s_msgpack, data4, 2);

	s_msgpack_negative_fixint_read(&s_msgpack, &i_var);
	printf("negative fixint: %d\n", i_var);

	/* int16 */
	char data5[] = { 0xd1, 0xa1, 0xff };
	s_msgpack_set_read_buf(&s_msgpack, data5, 4);
	int16_t i16_var = 0;
	s_msgpack_int16_read(&s_msgpack, &i16_var);
	printf("int16: %d\n", i16_var);

	/* uint16 */
	char data6[] = {0xcd, 0xa1, 0xff};
	s_msgpack_set_read_buf(&s_msgpack, data6, 4);
	uint16_t ui16_var = 0;
	s_msgpack_uint16_read(&s_msgpack, &ui16_var);
	printf("uint16: %d\n", ui16_var);

	/* int32 */
	char data7[] = {0xd2, 0xff, 0x11, 0x11, 0x11};
	s_msgpack_set_read_buf(&s_msgpack, data7, 6);
	int32_t i32_var = 0;
	s_msgpack_int32_read(&s_msgpack, &i32_var);
	printf("int32: %d\n", i32_var);

	/* uint32 */
	char data8[] = {0xce, 0xff, 0x11, 0x11, 0x11};
	s_msgpack_set_read_buf(&s_msgpack, data8, 6);
	uint32_t ui32_var = 0;
	s_msgpack_uint32_read(&s_msgpack, &ui32_var);
	printf("uint32: %u\n", ui32_var);

	/* float32 */
	char data9[] = {0xca, 0x40, 0xcc, 0xcc, 0xcc};
	s_msgpack_set_read_buf(&s_msgpack, data9, 6);
	float f_var = 0;
	s_msgpack_float32_read(&s_msgpack, &f_var);
	printf("float32: %f\n", f_var);

	/* Все чтения строк возвращают количество прочитанных байт(символов) в строке или 0 если ошибка или пустая строка */
	/* fixstr */
	char data10[] = { 0xA6, 0x73, 0x74, 0x72, 0x6F, 0x6B, 0x61 };
	s_msgpack_set_read_buf(&s_msgpack, data10, 8);
	char readed_str[BUFSIZE];
	result = s_msgpack_fixstr_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, fixstr: %s\n", result, readed_str);

	/* str8 */
	char data11[] = { 0xD9, 0x22, 0x73, 0x74, 0x72, 0x6F, 0x6B, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61 };
	s_msgpack_set_read_buf(&s_msgpack, data11, 37);
	result = s_msgpack_str8_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, str8: %s\n", result, readed_str);

	/* str16 */
	char data12[] = {0xDA, 0x01, 0x0A, 0x73, 0x74, 0x72, 0x6F, 0x6B, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};
	s_msgpack_set_read_buf(&s_msgpack, data12, 270);
	result = s_msgpack_str16_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, str16: %s\n", result, readed_str);

	/* str32 */
	char data13[] = {0xdb, 0x00, 0x00, 0x00, 0x03, 0x70, 0x71, 0x72};
	s_msgpack_set_read_buf(&s_msgpack, data13, 9);
	result = s_msgpack_str32_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, str32: %s\n", result, readed_str);

	/* true, false, neverused, nil */
	char data14[] = {0xc3};
	s_msgpack_set_read_buf(&s_msgpack, data14, 2);
	result = s_msgpack_true_read(&s_msgpack);
	printf("result: %u, true readed\n", result);

	char data15[] = {0xc2};
	s_msgpack_set_read_buf(&s_msgpack, data15, 2);
	result = s_msgpack_false_read(&s_msgpack);
	printf("result: %u, false readed\n", result);

	char data16[] = {0xc1};
	s_msgpack_set_read_buf(&s_msgpack, data16, 2);
	result = s_msgpack_neverused_read(&s_msgpack);
	printf("result: %u, neverused readed\n", result);

	char data17[] = {0xc0};
	s_msgpack_set_read_buf(&s_msgpack, data17, 2);
	result = s_msgpack_nil_read(&s_msgpack);
	printf("result: %u, nil readed\n", result);

	/* Все массивы и хеши возвращают количество элементов(пар ключ/значение) */
	/* fixarray */
	char data18[] = {0x93, 0x01, 0xA2, 0x68, 0x69, 0xC3};
	s_msgpack_set_read_buf(&s_msgpack, data18, 7);
	result = s_msgpack_fixarray_read(&s_msgpack);
	printf("fixarray count elems: %u\n", result);

	/* читаем дальше */
	s_msgpack_positive_fixint_read(&s_msgpack, &ui_var);
	printf("positive fixint in array: %u\n", ui_var);
	s_msgpack_fixstr_read(&s_msgpack, readed_str, 3);
	printf("fixstr in array: %s\n", readed_str);
	result = s_msgpack_true_read(&s_msgpack);
	printf("result: %u, true in array readed\n", result);

	/* также можем прочесть массив в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* array16 */
	char data19[] = {0xDC, 0x00, 0x02, 0x08, 0x03};
	s_msgpack_set_read_buf(&s_msgpack, data19, 6);
	result = s_msgpack_array16_read(&s_msgpack);
	printf("array16_read count elems: %u\n", result);
	/* также можем прочесть массив в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* array32 */
	char data20[] = {0xdd, 0x00, 0x00, 0x00, 0x03, 0x08, 0x09, 0x08};
	s_msgpack_set_read_buf(&s_msgpack, data20, 9);
	result = s_msgpack_array32_read(&s_msgpack);
	printf("array32_read count elems: %u\n", result);
	/* также можем прочесть массив в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* fixmap */
	char data21[] = {0x81, 0xa1, 0x32, 0x0c};
	s_msgpack_set_read_buf(&s_msgpack, data21, 5);
	result = s_msgpack_fixmap_read(&s_msgpack);
	printf("fixmap count pair key/val: %u\n", result);
	/* также можем прочесть хеш в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* map16 */
	char data22[] = {0xde, 0x00, 0x01, 0xA4, 0x6B, 0x65, 0x79, 0x31, 0x0c};
	s_msgpack_set_read_buf(&s_msgpack, data22, 10);
	result = s_msgpack_map16_read(&s_msgpack);
	printf("map16 count pair key/val: %u\n", result);
	/* также можем прочесть хеш в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* map32 */
	char data23[] = {0xdf, 0x00, 0x00, 0x00, 0x01, 0xA4, 0x6B, 0x64, 0x78, 0x31, 0x0f};
	s_msgpack_set_read_buf(&s_msgpack, data23, 12);
	result = s_msgpack_map32_read(&s_msgpack);
	printf("map32 count pair key/val: %u\n", result);
	/* также можем прочесть хеш в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* Смешенные типы */
	char data24[] = {0x97, 0xCa, 0x40, 0x34, 0x4C, 0xff, 0xCC, 0xDC, 0xCC, 0xCD, 0x82, 0xA3, 0x59, 0x45, 0x59, 0xCa, 0x21, 0x30, 0x2a, 0xff, 0xA3, 0x58, 0x58, 0x58, 0xA3, 0x53, 0x55, 0x4E, 0xC0, 0xC2, 0xC3};
	s_msgpack_set_read_buf(&s_msgpack, data24, 35);
	result = s_msgpack_fixarray_read(&s_msgpack);
	printf("fixarray count elems: %u\n", result);
	s_msgpack_float32_read(&s_msgpack, &f_var);
	printf("float32: %f\n", f_var);
	result = s_msgpack_uint8_read(&s_msgpack, &ui_var);
	printf("result: %u, uint8: %u\n", result, ui_var);
	result = s_msgpack_uint8_read(&s_msgpack, &ui_var);
	printf("result: %u, uint8: %u\n", result, ui_var);
	result = s_msgpack_fixmap_read(&s_msgpack);
	printf("fixmap count pair key/val: %u\n", result);
	result = s_msgpack_fixstr_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, fixstr: %s\n", result, readed_str);
	result = s_msgpack_float32_read(&s_msgpack, &f_var);
	printf("result: %u, float32: %e\n", result, f_var);
	result = s_msgpack_fixstr_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, fixstr: %s\n", result, readed_str);
	result = s_msgpack_fixstr_read(&s_msgpack, readed_str, BUFSIZE);
	printf("result: %u, fixstr: %s\n", result, readed_str);
	result = s_msgpack_nil_read(&s_msgpack);
	printf("result: %u, nil readed\n", result);
	result = s_msgpack_false_read(&s_msgpack);
	printf("result: %u, false readed\n", result);
	result = s_msgpack_true_read(&s_msgpack);
	printf("result: %u, true readed\n", result);

	/* также можем прочесть хеш в строку целиком */
	s_msgpack_set_to_start_read_buf(&s_msgpack);
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	/* примеры с переводом позиции буфера чтения */
	for (int i = 1; i < 13; i++) 
	{
		/* получение позиции i го элемента */
		uint32_t pos = s_msgpack_get_read_buf_pos_elem(&s_msgpack, i);
		printf("Elem: %u, Pos: %u\n", i, pos);

		/* передвижение позиции в указанную позицию */
		s_msgpack_set_to_pos_read_buf(&s_msgpack, pos);
		/* получение заголовка элемента на текущей позиции */
		uint8_t hr = s_msgpack_get_read_buf_header(&s_msgpack);

		/* перевод кода заголовка в строку и ее вывод */
		char mtype[128];
		s_msgpack_type_to_str(hr, mtype, 128);
		printf("%s\n", mtype);
	}

	/* примеры с установкой позиции к указанному элементу */
	for (int i = 1; i < 13; i++) 
	{
		/* установка позиции буфера к указанному элементу */
		s_msgpack_set_read_buf_pos_to_elem(&s_msgpack, i);
		/* получение загловка элемента */
		uint8_t hr = s_msgpack_get_read_buf_header(&s_msgpack);
		/* перевод в строку, вывод на экран */
		char mtype[128];
		s_msgpack_type_to_str(hr, mtype, 128);
		printf("%s\n", mtype);
	}

	/* получить тип N элемента */
	uint8_t elem_type = s_msgpack_read_elem(&s_msgpack, 10);
	char mtype[128];
	s_msgpack_type_to_str(elem_type, mtype, 128);
	printf("%s\n", mtype);

	/* установка позиции буфера к указанному элементу */
	s_msgpack_set_read_buf_pos_to_elem(&s_msgpack, 2);
	/* ВАЖНО s_msgpack_read_to_str читает элементы последовательно, 
	 и если первый элемент имеет тип массива или хеш,
	 то прочитает все его содержимое, 
	 иначе просто прочитает один элемент 
	*/
	s_msgpack_read_to_str(&s_msgpack, readed_str, BUFSIZE);
	printf("%s\n", readed_str);

	// char data25[] = {0x01, 0x00};
	// s_msgpack_set_read_buf(&s_msgpack, data25, 3);
	// uint32_t rrr;
	// s_msgpack_uint_read(&s_msgpack,&rrr);
	// printf("rrr: %d\n",rrr);

	
	/*** Примеры с записью msgpack сообщений ***/
	
	s_msgpack_array_write(&s_msgpack, 2);
	s_msgpack_fixstr_write(&s_msgpack,"hubro",5);
	s_msgpack_map_write(&s_msgpack,9);
	s_msgpack_fixstr_write(&s_msgpack,"1",1);
	s_msgpack_positive_fixint_write(&s_msgpack, 1);
	s_msgpack_fixstr_write(&s_msgpack,"2",1);
	s_msgpack_positive_fixint_write(&s_msgpack, 2);
	s_msgpack_fixstr_write(&s_msgpack,"3",1);
	s_msgpack_positive_fixint_write(&s_msgpack, 3);
	s_msgpack_fixstr_write(&s_msgpack,"4",1);
	s_msgpack_array_write(&s_msgpack, 0);
	s_msgpack_fixstr_write(&s_msgpack,"5",1);
	s_msgpack_int_write(&s_msgpack, -12);
	s_msgpack_fixstr_write(&s_msgpack,"6",1);
	s_msgpack_bool_write(&s_msgpack, 0);
	s_msgpack_str_write(&s_msgpack,"ggwp",4);
	s_msgpack_bool_write(&s_msgpack, 1);
	s_msgpack_fixstr_write(&s_msgpack,"8",1);
	s_msgpack_nil_write(&s_msgpack);
	s_msgpack_fixstr_write(&s_msgpack,"10",2);
	s_msgpack_float_write(&s_msgpack, 1.222);

	for (int j = 0; j < s_msgpack_get_write_buf_pos(&s_msgpack); j++)
	{
		printf("0x%02x ", (unsigned char)s_msgpack_get_write_buf_pos_byte(&s_msgpack,j));
	}
	printf("\n");

	s_msgpack_set_read_buf(&s_msgpack,buffer_write,BUFSIZE);
	s_msgpack_read_to_str(&s_msgpack,readed_str,BUFSIZE);
	printf("%s\n", readed_str);
	return 0;
}