#include "present.h"

#define BUFSIZE 128
#define BIT_COUNT 8

static char buffer[BUFSIZE] = {'\0'};
static char seek = 0;

static void parse_byte(uint8_t *byte);
static void print_bits(uint8_t byte_count);

/* Парсим каждый байт числа */
static void parse_byte(uint8_t * byte)
{
	char bit, *ptr;
	ptr = buffer;
	for (int i = 0; i < BIT_COUNT; i++)
	{
		bit = (0x1 & (*byte >> i));
		bit == 1 ? (bit = '1') : (bit = '0');
		*(ptr + seek + (BIT_COUNT - i) - 1) = bit;
	}
}

/* Выводим биты */
static void print_bits(uint8_t byte_count)
{
	int counter = 0;
	for (int i = 0; i < (byte_count * BIT_COUNT); i++)
	{
		if ((counter % BIT_COUNT) == 0 && counter != 0)
		{
			printf("_");
		}
		printf("%c", buffer[i]);
		counter++;
	}
	printf("\n");
}

/* Получаем биты строки\числа */
void get_bits(void *number, uint8_t byte_count)
{
	memset(buffer, '\0', BUFSIZE);
	uint8_t *ptr = (uint8_t *)number;
	seek = (byte_count - 1) * BIT_COUNT;
	for (int i = 0; i < byte_count; i++)
	{
		parse_byte(ptr + i);
		seek -= BIT_COUNT;
	}
	print_bits(byte_count);
}

/* Получаем hex код строки */
void get_hex_string(char *msg)
{
	char *ptr = msg;
	while (*ptr)
	{
		printf("0x%02x ", *ptr);
		ptr++;
	}
	printf("\n");
}
