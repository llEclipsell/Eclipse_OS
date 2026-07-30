#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define FLAG_LEFT  0x01  // '-'
#define FLAG_SIGN  0x02  // '+'
#define FLAG_SPACE 0x04  // ' '
#define FLAG_TYPE  0x08  // '#'
#define FLAG_ZERO  0x10  // '0'

static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

static bool print_number(uintmax_t value, unsigned base, bool uppercase,
                         bool negative, int* written, size_t maxrem) {
	char buf[32];
	size_t i = 0;
	const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

	if (value == 0) {
		buf[i++] = '0';
	} else {
		while (value != 0) {
			buf[i++] = digits[value % base];
			value /= base;
		}
	}

	if (negative)
		buf[i++] = '-';

	if (maxrem < i)
		return false;

	/* Digits were generated least-significant first; reverse in place. */
	for (size_t j = 0; j < i / 2; j++) {
		char t = buf[j];
		buf[j] = buf[i - 1 - j];
		buf[i - 1 - j] = t;
	}

	if (!print(buf, i))
		return false;

	*written += i;
	return true;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		int flags = 0;
		while (*format == '-' || *format == '+' || *format == ' ' ||
		       *format == '#' || *format == '0') {
			switch (*format) {
				case '-': flags |= FLAG_LEFT;  break;
				case '+': flags |= FLAG_SIGN;  break;
				case ' ': flags |= FLAG_SPACE; break;
				case '#': flags |= FLAG_TYPE;  break;
				case '0': flags |= FLAG_ZERO;  break;
			}
			format++;
		}

		int width = 0;
		while (*format >= '0' && *format <= '9')
			width = width * 10 + (*format++ - '0');

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		}
		else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'd' || *format == 'i') {
			format++;
			int value = va_arg(parameters, int);
			bool negative = value < 0;
			uintmax_t uvalue = negative ? -(uintmax_t) value : (uintmax_t) value;
			if (!print_number(uvalue, 10, false, negative, &written, maxrem))
				return -1;
		}
		else if (*format == 'u') {
			format++;
			unsigned int value = va_arg(parameters, unsigned int);
			if (!print_number(value, 10, false, false, &written, maxrem))
				return -1;
		}
		else if (*format == 'x' || *format == 'X') {
			bool uppercase = (*format == 'X');
			format++;
			unsigned int value = va_arg(parameters, unsigned int);
			if (!print_number(value, 16, uppercase, false, &written, maxrem))
				return -1;
		}
		else if (*format == 'o') {
			format++;
			unsigned int value = va_arg(parameters, unsigned int);
			if (!print_number(value, 8, false, false, &written, maxrem))
				return -1;
		}
		else if (*format == 'p') {
			format++;
			uintptr_t value = (uintptr_t) va_arg(parameters, void*);
			if (maxrem < 2 || !print("0x", 2))
				return -1;
			written += 2;
			if (!print_number(value, 16, false, false, &written, maxrem - 2))
				return -1;
		}
		else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}

