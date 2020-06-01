#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_byte(FILE *input, uint8_t *b)
{
	int result = fgetc(input);

	if (result == EOF)
	{
		if (!feof(input))
		{
			fprintf(stderr, "Failed to read from input.");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	*b = (uint8_t)result;
	return 1;
}

static void write_byte(FILE *output, uint8_t b)
{
	if (fputc(b, output) == EOF)
	{
		fprintf(stderr, "Failed to write to output.");
		exit(EXIT_FAILURE);
	}
}

void loding_bar(long int anzahl, long int ges)
{
	float pr = (float)anzahl / (float)ges * 100;
	fprintf(stderr, "\r");
	/*fprintf(stderr, "[");
	int i;
	for (i = 1; i <= ((int)pr / 5); i++)
	{
		fprintf(stderr, "#");
	}
	for (i = i; i <= 20; i++)
	{
		fprintf(stderr, "-");
	}*/
	fprintf(stderr, "]  %.0f%%", pr);
	//fflush(stderr);
}

typedef enum __state_t__
{
	STATE_IND,
	STATE_HOM,
	STATE_HET,
} state_t;

int main(int argc, char **argv)
{
	FILE *input = stdin;
	FILE *output = stdout;

	if (input == NULL || output == NULL)
	{
		fprintf(stderr, "Can not open the file!\n");
	}

	if (argc > 2)
	{
		fprintf(stderr, "Too many arguments!\n usage: \.honkpack -c|-u <input >output\n");
	}
	if (argc < 2)
	{
		fprintf(stderr, "Too few arguments!\n usage: \.honkpack -c|-u <input >output\n ");
	}

	fseek(input, 0L, SEEK_END);

	// calculating the size of the file
	long int ges = ftell(input);
	fprintf(stderr, "%li", ges);

	fseek(input, 0L, SEEK_SET);

	//State machine:
	uint8_t b = 0x00;
	size_t count = 0;
	state_t state = STATE_IND;
	long int byte_count = 0;

	if (strcmp(argv[1], "-u") == 0)
	{
		while (read_byte(input, &b))
		{
			switch (state)
			{
			case STATE_IND:

				count = (size_t)(b & 0x7F); //Binary: 0b01111111
				state = (b >> 7) ? STATE_HOM : (count > 0) ? STATE_HET : STATE_IND;

				break;

			case STATE_HOM:

				for (size_t i = 0; i < count; i++)
				{
					write_byte(output, b);
				}

				state = STATE_IND;
				break;

			case STATE_HET:

				write_byte(output, b);

				if (--count == 0)
				{
					state = STATE_IND;
				}

				break;
			}
		}
	}
	else if (strcmp(argv[1], "-c") == 0)
	{
		uint8_t a_byte[127];
		uint8_t honk_byte = 0x00;
		while (read_byte(input, &b))
		{
			byte_count++;
			loding_bar(byte_count, ges);
			switch (state)
			{
			case STATE_IND:
				if (count > 0)
				{
					if (a_byte[count - 1] == b)
						state = STATE_HOM;

					else
						state = STATE_HET;
				}
				count++;
				a_byte[count - 1] = b;
				break;

			case STATE_HOM:

				if (b != a_byte[0] || count == 127)
				{
					state = STATE_IND;
					honk_byte = 0x80 | (uint8_t)count;
					write_byte(output, honk_byte);
					write_byte(output, a_byte[0]);

					count = 1;
					a_byte[0] = b;
				}
				else
				{
					count++;
				}

				break;

			case STATE_HET:

				if (b != a_byte[count - 1] && count < 127)
				{
					a_byte[count] = b;
					count++;
				}
				else
				{
					honk_byte = 0x00 | (uint8_t)count;
					write_byte(output, honk_byte);
					for (int i = 0; i < count; i++)
						write_byte(output, a_byte[i]);
					count = 1;
					a_byte[count - 1] = b;
					state = STATE_IND;
				}
				break;
			}
		}
		switch (state)
		{

		case STATE_IND:
			break;

		case STATE_HOM:

			honk_byte = 0x80 | (uint8_t)count;
			write_byte(output, honk_byte);
			write_byte(output, a_byte[count - 1]);
			break;

		case STATE_HET:

			honk_byte = (uint8_t)count;
			write_byte(output, honk_byte);
			for (int i = 0; i < count; i++)
				write_byte(output, a_byte[i]);
			count = 1;

			break;
		}
	}
	else
	{
		fprintf(stderr, "wrong argument!\n \.honkpack -c|-u <input >output\n");
	}

	//Close the files:
	fclose(input);
	fclose(output);
}
