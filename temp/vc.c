/*
Objetivo:
  Reconhecer e pintar numeros/letras e placa de uma matricula

Trabalho Realizado por:
  a17616  - José Rodrigues
  a12314  - Sérgio Santos
  a4561   - Ulisses Ferreira
*/

#define _CRT_SECURE_NO_WARNINGS

#pragma region Includes

//Includes do sistema
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
// Inlcudes
#include "vc.h"

#pragma endregion

#pragma region Variables
//Pasta onde serão guardados os resultados obtidos
const char results[50] = "results/";

#define PI 3.14159265358979323846
#pragma endregion

#pragma region Funcoes ALOCAR E LIBERTAR UMA IMAGEM

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("============================== IMAGEM ABERTA ==================================\n");
			printf("Imagem: %d x %d | Channels: %d | Levels: %d\n", image->width, image->height, image->channels, image->levels);
			printf("Localização: %s", filename);
			printf("\n===============================================================================\n\n");
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("============================== IMAGEM ABERTA ==================================\n");
			printf("Imagem: %d x %d | Channels: %d | Levels: %d\n", image->width, image->height, image->channels, image->levels);
			printf("Localização: %s", filename);
			printf("\n===============================================================================\n\n");
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}
		printf("============================== IMAGEM GERADA ==================================\n");
		printf("Imagem: %d x %d | Channels: %d | Levels: %d\n", image->width, image->height, image->channels, image->levels);
		printf("Localização: %s\n", filename);
		printf("===============================================================================\n\n");
		fclose(file);

		return 1;
	}

	return 0;
}

#pragma endregion

#pragma region Save and Run

char *conc(const char *first, char *second)
{
	char *aux = _strdup(first);
	strcat(aux, second);
	return aux;
}

void save(char *filename, IVC *image)
{
	char *filepath = conc(results, filename);
	vc_write_image(filepath, image);
}

#pragma endregion

#pragma region RGB to GRAY

int vc_rgb_gray(IVC *original, IVC *converted)
{
	int pos, posAux, x, y;
	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			posAux = y * converted->bytesperline + x * converted->channels;
			converted->data[posAux] = ((0.3 * original->data[pos] + 0.59 * original->data[pos + 1] + 0.11 * original->data[pos + 2]) / 3);
		}
	}

	return 0;
}

#pragma endregion

#pragma region Manuseamento de channels

int vc_only_red(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			converted->data[pos] = original->data[pos];
			converted->data[pos + 1] = original->data[pos];
			converted->data[pos + 2] = original->data[pos];
		}
	}

	return 0;
}

int vc_only_green(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			converted->data[pos + 1] = original->data[pos + 1];
			converted->data[pos] = 0;
			converted->data[pos + 2] = 0;
		}
	}

	return 0;
}

int vc_only_blue(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			converted->data[pos + 2] = original->data[pos + 2];
			converted->data[pos + 1] = 0;
			converted->data[pos] = 0;
		}
	}

	return 0;
}

int vc_remove_red(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;

			converted->data[pos] = 0;
			converted->data[pos + 1] = original->data[pos + 1];
			converted->data[pos + 2] = original->data[pos + 2];
		}
	}
	return 0;
}

int vc_remove_green(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			converted->data[pos] = original->data[pos];
			converted->data[pos + 1] = 0;
			converted->data[pos + 2] = original->data[pos + 2];
		}
	}
	return 0;
}

int vc_remove_blue(IVC *original, IVC *converted)
{
	int pos, x, y;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			converted->data[pos] = original->data[pos];
			converted->data[pos + 1] = original->data[pos + 1];
			converted->data[pos + 2] = 0;
		}
	}

	return 0;
}

#pragma endregion

#pragma region Binary

float vc_media(IVC *original)
{
	int pos, x, y;
	float media = 0;

	if (original->channels == 1)
	{
		for (x = 0; x < original->width; x++)
			for (y = 0; y < original->height; y++)
			{
				pos = y * original->bytesperline + x * original->channels;
				media += original->data[pos];
			}
	}
	else if (original->channels == 3)
	{
		for (x = 0; x < original->width; x++)
			for (y = 0; y < original->height; y++)
			{
				pos = y * original->bytesperline + x * original->channels;
				media += (original->data[pos] + original->data[pos + 1] + original->data[pos + 2]) / 3;
			}
	}
	return (media / (original->bytesperline * original->height));
}

int vc_rgb_gray_to_binary_global_mean(IVC *original, IVC *converted)
{
	int pos, posAux, x, y;
	float media = vc_media(original), meanAux = 0;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			posAux = y * converted->bytesperline + x * converted->channels;
			meanAux = (original->channels == 1 ? original->data[pos] : (original->data[pos] + original->data[pos + 1] + original->data[pos + 2])) / original->channels;
			converted->data[posAux] = meanAux > media ? 255 : 0;
		}
	}

	return 1;
}

int vc_rgb_gray_to_binary(IVC *original, IVC *converted, int threshold)
{
	int pos, posAux, x, y;
	float meanAux;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			posAux = y * converted->bytesperline + x * converted->channels;
			meanAux = (original->channels == 1 ? original->data[pos] : (original->data[pos] + original->data[pos + 1] + original->data[pos + 2])) / original->channels;
			converted->data[posAux] = meanAux > threshold ? 255 : 0;
		}
	}

	return 1;
}

int vc_gray_to_binary_bernsen(IVC *original, IVC *converted, int kernel, int c)
{
	long int pos, posk;
	float meanAux;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = (kernel - 1) / 2;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	int max = 0, min = 255;
	float threshold;
	int count = 0;

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL))
		return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			min = 255;
			max = 0;
			count = 0;
			//
			for (ky = -offset; ky <= offset; ky++)
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (original->data[posk] > max)
							max = original->data[posk];
						if (original->data[posk] < min)
							min = original->data[posk];
					}
				}

			//
			if ((max - min) < c)
				threshold = 255 / 2;
			else
				threshold = (unsigned char)((float)(max + min) / (float)2);

			if (original->data[pos] > threshold)
				converted->data[pos] = 255;
			else
				converted->data[pos] = 0;
		}

	return 1;
}

int vc_binary_dilate(IVC *original, IVC *converted, int kernel)
{
	long int pos, posk;
	float meanAux;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = (kernel - 1) / 2;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	float threshold;

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL))
		return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			//
			converted->data[pos] = 0;
			for (ky = -offset; ky <= offset; ky++)
				for (kx = -offset; kx <= offset; kx++)
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;
						//
						if (original->data[posk] == 255)
							converted->data[pos] = 255;
					}
		}

	return 1;
}

int vc_binary_erode(IVC *original, IVC *converted, int kernel)
{
	long int pos, posk;
	float meanAux;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = (kernel - 1) / 2;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	float threshold;

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL))
		return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels))
		return 0;
	if (channels != 1)
		return 0;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			//
			converted->data[pos] = 255;
			for (ky = -offset; ky <= offset; ky++)
				for (kx = -offset; kx <= offset; kx++)
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;
						//
						if (original->data[posk] == 0)
							converted->data[pos] = 0;
					}
		}

	return 1;
}

int vc_binary_open(IVC *original, IVC *converted, int kernel)
{
	IVC *temp;
	temp = vc_image_new(original->width, original->height, 1, 255);

	//Erode
	vc_binary_erode(original, temp, kernel);
	//Dilate
	vc_binary_dilate(temp, converted, kernel);

	return 1;
}

int vc_binary_close(IVC *original, IVC *converted, int kernel)
{
	IVC *temp;
	temp = vc_image_new(original->width, original->height, 1, 255);

	//Dilate
	vc_binary_dilate(original, temp, kernel);
	//Erode
	vc_binary_erode(temp, converted, kernel);

	return 1;
}

#pragma endregion

#pragma region Labeling
// Etiquetagem de blobs
// src		: Imagem binaria de entrada
// dst		: Imagem grayscale (ira conter as etiquetas)
// nlabels	: Endereco de memoria de uma varievel, onde sera armazenado o numero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necessario libertar posteriormente esta memoria.
OVC* vc_binary_blob_labelling(IVC* original, IVC* converted, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)original->data;
	unsigned char* datadst = (unsigned char*)converted->data;
	int width = original->width;
	int height = original->height;
	int bytesperline = original->bytesperline;
	int channels = original->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 }; //Primeiro elemento do array = 0
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verificao de erros
	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL)) return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binaria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixeis de plano de fundo devem obrigatoriamente ter valor 0
	// Todos os pixeis de primeiro plano devem obrigatoriamente ter valor 255
	// Seria atribuidas etiquetas no intervalo [1,254]
	// Este algoritmo esta assim limitado a 255 labels
	//Default do array de destino
	for (i = 0, size = bytesperline * height; i < size; i++)
		if (datadst[i] != 0)
			datadst[i] = 255;

	// Limpa os rebordos da imagem binaria (deixa de ser necessário verificar se o kernel esta dentro da imagem)
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X


			if (label > 255)
			{
				(*nlabels) = -1;
				return NULL;
			}
				
													// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					//getchar();
					// Se A esta marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B esta marcado, e o menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C esta marcado, e o menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D esta marcado, e o menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
						if (labeltable[datadst[posA]] != num)
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
								if (labeltable[a] == tmplabel)
									labeltable[a] = num;

					if (datadst[posB] != 0)
						if (labeltable[datadst[posB]] != num)
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
								if (labeltable[a] == tmplabel)
									labeltable[a] = num;

					if (datadst[posC] != 0)
						if (labeltable[datadst[posC]] != num)
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
								if (labeltable[a] == tmplabel)
									labeltable[a] = num;

					if (datadst[posD] != 0)
						if (labeltable[datadst[posD]] != num)
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
								if (labeltable[a] == tmplabel)
									labeltable[a] = num;
				}
			}
		}


	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
		for (x = 1; x < width - 1; x++) {
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
				datadst[posX] = labeltable[datadst[posX]];
		}

	// Contagem do numero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}

	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}


	// Se nao ha blobs
	if (*nlabels == 0) return NULL;



	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
		for (a = 0; a < (*nlabels); a++)
			blobs[a].label = labeltable[a];
	else
		return NULL;

	printf("Nº blobls: %d\n", *nlabels);

	if (*nlabels > 200)
		return NULL;
	//
	return blobs;
}

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
						blobs[i].perimeter++;
				}
			}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

#pragma endregion

#pragma region Edge

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	int histmax, histthreshold;
	int sumx, sumy;
	int hist[256] = {0};

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
		return 0;
	if (channels != 1)
		return 0;

	size = width * height;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			datadst[posX] = (unsigned char)sqrt((double)(sumx * sumx + sumy * sumy));
		}
	}

	// Compute a grey level histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			hist[datadst[y * bytesperline + x * channels]]++;
		}
	}

	// Threshold at the middle of the occupied levels
	histmax = 0;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th))
			break;
	}
	histthreshold = i;

	// Apply the threshold
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		{
			posX = y * bytesperline + x * channels;

			if (datadst[posX] >= histthreshold)
				datadst[posX] = 255;
			else
				datadst[posX] = 0;
		}

	return 1;
}

int vc_binary_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001 a 1.000]
{

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	int histmax, histthreshold;
	int sumx, sumy;
	int hist[256] = {0};

	IVC *dest = vc_image_new(dst->width, dst->height, dst->channels, dst->levels);

	//verificao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height))
		return 0;
	if ((src->channels != 1) || (dst->channels != 1))
		return 0;

	size = width * height;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			//         _______________________
			//        |     |    |    |
			//        | POS_A | POS_B | POS_C |
			//        |  -1  |  -1  |  1  |
			//        |_______|_______|_______|
			//        |     |    |    |
			//        | POS_D | POS_X | POS_E |
			//        |  -1  |   0  |  1  |
			//        |_______|_______|_______|
			//        |     |    |    |
			//        | POS_F | POS_G | POS_H |
			//        |  -1  |   1  |  1  |
			//        |_______|_______|_______|

			// Convolução
			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1
			// magnitude = 3

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1
			// magnitude = 3

			// Calcular a magnitude do vector:
			datadst[posX] = (unsigned char)sqrt((double)(sumx * sumx + sumy * sumy));
			//datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / 4.0);
		}
	}

	// Compute a grey level histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			hist[datadst[y * bytesperline + x * channels]]++;
		}
	}

	// Threshold at the middle of the occupied levels
	histmax = 0;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th))
			break;
	}
	histthreshold = i;
	//vc_gray_to_binary_bernsen(datadst, , <#int kernel#>, <#int c#>)

	// Apply the threshold
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			posX = y * bytesperline + x * channels;

			// Aplica threshold: -> Se a Magnitude(x,y) > th entao é pixel de contorno
			// datadst[posX] = Magnitude  &&  histthreshold = th
			if (datadst[posX] >= histthreshold)
				datadst[posX] = 255;
			else
				datadst[posX] = 0;
		}
	}
	return 1;
}

int vc_binary_edge_sobel(IVC* src, IVC* dst, float th) // th = [0.001 a 1.000]
{

	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	int histmax, histthreshold;
	int sumx, sumy;
	int hist[256] = { 0 };

	//verificao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	size = width * height;


	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 2) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 2) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			//         _______________________
			//        |     |    |    |
			//        | POS_A | POS_B | POS_C |
			//        |  -1  |  -2  |  1  |
			//        |_______|_______|_______|
			//        |     |    |    |
			//        | POS_D | POS_X | POS_E |
			//        |  -2  |   0  |  2  |
			//        |_______|_______|_______|
			//        |     |    |    |
			//        | POS_F | POS_G | POS_H |
			//        |  -1  |   2  |  1  |
			//        |_______|_______|_______|

					// Convolução
			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -2;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +2;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1
			// magnitude = 3

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -2;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +2;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1
			// magnitude = 3

			// Calcular a magnitude do vector:
			datadst[posX] = (unsigned char)sqrt((double)(sumx * sumx + sumy * sumy));
			//datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / 4.0);
		}
	}

	// Compute a grey level histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			hist[datadst[y * bytesperline + x * channels]]++;
		}
	}

	// Threshold at the middle of the occupied levels
	histmax = 0;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i;

	// Apply the threshold
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			posX = y * bytesperline + x * channels;

			// Aplica threshold: -> Se a Magnitude(x,y) > th entao é pixel de contorno
			// datadst[posX] = Magnitude  &&  histthreshold = th
			if (datadst[posX] >= histthreshold) datadst[posX] = 255;
			else datadst[posX] = 0;
		}
	}
	return 0;
}

#pragma endregion

#pragma region Histograma

float* vc_histogram_array(IVC* original) {
	//VARS
	int pos, y, x, max = original->levels, maior = 0;
	float* array;

	//Inicializa array
	array = (float *)malloc(max + 1);
	array[0] = 0;
	for (int i = 0; i < max; i++) array[i] = 0;

	//Gera vetor do histogram
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] != 0)
				array[original->data[pos]]++;
		}

	return array;
}

IVC* vc_gray_histogram_show(IVC* original) {
	//VARS
	int pos, y, x, max = original->levels + 1, maior = 0;
	float* array;
	int n = original->width * original->height;
	IVC* converted;
	converted = vc_image_new(255, 350, 1, 255);

	array = vc_histogram_array(original);

	//Calcular index com maior valor
	for (int i = 1; i < max; i++) {
		if (array[i] > maior) maior = array[i];
	}

	//coloca os valores entre 0-300
	for (int i = 1; i < max; i++) {
		array[i] = (array[i] * 300) / maior;
	}
	printf("0 : %d , 255 : %d\n", array[0], array[255]);

	//Paint
	for (y = 0; y < converted->height; y++)
		for (x = 0; x < converted->width; x++)
		{
			pos = y * converted->bytesperline + x * converted->channels;
			//Reset
			converted->data[pos] = 0;

			//Paint
			if (y > 20 && y <= 320 && x <= 255) {
				if ((320 - y) > array[x]) {
					converted->data[pos] = 255;
				}
			}
			else if (y > 320)
				converted->data[pos] = x;
			else
				converted->data[pos] = 255;
		}
	return converted;
}

int vc_rgb_histogram_show(IVC* original, IVC* converted) {
	//VARS
	int pos, y, x, max = original->levels + 1, i;
	float *r, *g, *b;
	int maior_r = 0, maior_g = 0, maior_b = 0;
	int n = original->width * original->height;
	r = (float*)malloc(max * sizeof(float));
	g = (float*)malloc(max * sizeof(float));
	b = (float*)malloc(max * sizeof(float));

	//Inicializa array
	r[0] = 0;
	g[0] = 0;
	b[0] = 0;
	for (i = 1; i < max; i++) {
		r[i] = 0;
		g[i] = 0;
		b[i] = 0;
	}

	//Gera vetor do histogram
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] != 0) r[original->data[pos]]++;
			if (original->data[pos + 1] != 0) g[original->data[pos + 1]]++;
			if (original->data[pos + 2] != 0) b[original->data[pos + 2]]++;
		}

	//Calcular index com maior valor
	for (int i = 1; i < max; i++) {
		if (r[i] > maior_r) maior_r = r[i];
		if (g[i] > maior_g) maior_g = g[i];
		if (b[i] > maior_b) maior_b = b[i];
	}

	//coloca os valores entre 0-300
	for (i = 1; i < max; i++) {
		r[i] = (r[i] * 300) / maior_r;
		g[i] = (g[i] * 300) / maior_g;
		b[i] = (b[i] * 300) / maior_b;
	}

	//Paint
	for (y = 0; y < converted->height; y++)
		for (x = 0; x < converted->width; x++)
		{
			pos = y * converted->bytesperline + x * converted->channels;

			//Paint
			//Red
			if (x == 255 || x == 510) {
				converted->data[pos] = 0;
				converted->data[pos + 1] = 0;
				converted->data[pos + 2] = 0;
			}
			else if (y <= 300 && x <= 255) {
				if ((320 - y) > r[x]) {
					converted->data[pos] = 255;
					converted->data[pos + 1] = 255;
					converted->data[pos + 2] = 255;
				}
			}
			else if (y > 300 && x <= 255) { //Bottom
				converted->data[pos] = x;
				converted->data[pos + 1] = 0;
				converted->data[pos + 2] = 0;
			}

			// Green
			else if (y <= 300 && x > 255 && x <= 510) {
				if ((320 - y) > g[x - 255] && x > 255 && x <= 510) {
					converted->data[pos] = 255;
					converted->data[pos + 1] = 255;
					converted->data[pos + 2] = 255;
				}
			}
			else if (y > 300 && x > 255 && x <= 510) { //Bottom
				converted->data[pos] = 0;
				converted->data[pos + 1] = x;
				converted->data[pos + 2] = 0;
			}

			// Blue
			else if (y <= 300 && x > 510 && x <= 765) {
				if ((300 - y) > b[x - 510] && x > 510 && x <= 765) {
					converted->data[pos] = 255;
					converted->data[pos + 1] = 255;
					converted->data[pos + 2] = 255;
				}
			}
			else if (y > 300 && x > 510 && x <= 765) { //Bottom
				converted->data[pos] = 0;
				converted->data[pos + 1] = 0;
				converted->data[pos + 2] = x;
			}
		}
	return 1;
}

int vc_gray_histogram_equalization(IVC* original, IVC* converted) {
	//VARS
	int pos, y, x, max = original->levels;
	float* array;
	int n = original->width * original->height;

	//Vetor de histogram
	array = vc_histogram_array(original);

	//Calcular vetor da acumulada
	for (int i = 1; i < max; i++) {
		array[i] = array[i - 1] + ((array[i] / n) * original->levels);
	}

	//Correr a imagem para atribuir a acumulada
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] != 0)
				converted->data[pos] = array[original->data[pos]];
		}

	//
	return 1;
}

int vc_rgb_histogram_equalization(IVC* original, IVC* converted) {
	//VARS
	int pos, y, x, max = original->levels + 1, i;
	float *r, *g, *b;
	int n = original->width * original->height;
	r = (float*)malloc(max * sizeof(float));
	g = (float*)malloc(max * sizeof(float));
	b = (float*)malloc(max * sizeof(float));

	//Inicializa array
	r[0] = 0;
	g[0] = 0;
	b[0] = 0;
	for (i = 1; i < max; i++) {
		r[i] = 0;
		g[i] = 0;
		b[i] = 0;
	}

	//Gera vetor do histogram
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] != 0) r[original->data[pos]]++;
			if (original->data[pos + 1] != 0) g[original->data[pos + 1]]++;
			if (original->data[pos + 2] != 0) b[original->data[pos + 2]]++;
		}

	//Calcular vetor da acumulada
	for (i = 1; i < max; i++) {
		r[i] = r[i - 1] + ((r[i] / n) * original->levels);
		g[i] = g[i - 1] + ((g[i] / n) * original->levels);
		b[i] = b[i - 1] + ((b[i] / n) * original->levels);
	}

	//Correr a imagem para atribuir a acumulada
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] != 0) converted->data[pos] = r[original->data[pos]];
			if (original->data[pos + 1] != 0) converted->data[pos + 1] = g[original->data[pos + 1]];
			if (original->data[pos + 2] != 0) converted->data[pos + 2] = b[original->data[pos + 2]];
		}
	return 1;
}
#pragma endregion

#pragma region Filters

int vc_gray_lowpass_mean_filter(IVC *original, IVC *converted)
{
	long int pos, posk, count = 0;
	float meanAux;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = 1;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	float threshold;
	int mask[9] = {1, 1, 1,
				   1, 1, 1,
				   1, 1, 1};

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL))
		return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels))
		return 0;
	if (channels != 1)
		return 0;

	//Copy borders from original to converted
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
			if (y == 0 || y == (original->height - 1) || x == 0 || x == (original->width - 1))
				converted->data[(y * original->width) + x] = original->data[(y * original->width) + x];

	//Apply mask
	for (y = 1; y < original->height; y++)
		for (x = 1; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			count = 0;
			meanAux = 0;
			//
			for (ky = -offset; ky <= offset; ky++)
				for (kx = -offset; kx <= offset; kx++)
				{
					posk = (y + ky) * bytesperline + (x + kx) * channels;
					//
					meanAux += original->data[posk] * mask[count++];
				}
			//
			meanAux /= 9;
			converted->data[pos] = meanAux;
		}

	return 0;
}

int vc_gray_lowpass_median_filter(IVC* original, IVC* converted)
{
	long int pos, posk, count = 0;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = 1;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	float threshold;
	int mask[9] = { 1, 1, 1,
				   1, 1, 1,
				   1, 1, 1 };
	int array[9];

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL))
		return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels))
		return 0;
	if (channels != 1)
		return 0;

	//Copy borders from original to converted
	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
			if (y == 0 || y == (original->height - 1) || x == 0 || x == (original->width - 1))
				converted->data[(y * original->width) + x] = original->data[(y * original->width) + x];

	//Apply mask
	for (y = 1; y < original->height; y++)
		for (x = 1; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			count = 0;
			//
			for (ky = -offset; ky <= offset; ky++)
				for (kx = -offset; kx <= offset; kx++)
				{
					posk = (y + ky) * bytesperline + (x + kx) * channels;
					//
					array[count++] = original->data[posk];
				}
			//
			for (int i = 0; i < 9; i++)    
				for (int j = 0; j < 9; j++)     
					if (array[j] > array[i])               
					{
						int tmp = array[i];   
						array[i] = array[j];
						array[j] = tmp;
					}

			converted->data[pos] = array[4];
		}

	return 0;
}

#pragma endregion

#pragma region Trabalho
//Fase 1

int vc_trab_fase1(IVC* original)
{
	//VARS
	int detected_y = -1;
	IVC* dilate, * prewitt; //GRAY

	//Inicialize
	dilate = vc_image_new(original->width, original->height, 1, 255);
	prewitt = vc_image_new(original->width, original->height, 1, 255);

	//prewitt
	vc_gray_edge_prewitt(original, prewitt, 0.970);
	//vc_write_image("fase1_prewitt.pgm", prewitt);

	//Dilate
	vc_binary_close(prewitt, dilate, 3);
	//vc_write_image("fase1_dilate.pgm", dilate);

	//Detecta a linha da matricula e retorna o valor
	detected_y = vc_trab_detect(dilate);

	//Retorna valor do y detectado
	return detected_y;
}

OVC *vc_trab_fase2(IVC *original, IVC *license, int *detected_line)
{

	//VARS
	IVC *prewitt, *filter, *clean;
	int nLabels = 0, max = 0, index = -1, i = 0;
	int x, y, pos;

	//Inicialize
	clean = vc_image_new(original->width, original->height, 1, 255);
	prewitt = vc_image_new(original->width, original->height, 1, 255);
	filter = vc_image_new(original->width, original->height, 1, 255);

	//Clean
	vc_trab_clean(original, clean, detected_line, 13);
	//vc_write_image("clean.pgm", clean);

	//Prewitt
	vc_binary_edge_prewitt(clean, prewitt, 0.983);
	//vc_write_image("fase2_prewitt.pgm", prewitt);

	//Filter Mean
	vc_gray_lowpass_mean_filter(prewitt, filter);
	//vc_write_image("fase2_filter.pgm", filter);

	//Get Blobs
	OVC *blobs = vc_binary_blob_labelling(filter, license, &nLabels);
	if (nLabels == -1)
	{
		detected_line = -1;
		return NULL;
	}
	vc_binary_blob_info(license, blobs, nLabels);

	//Encontra a matricula
	for (i = 0; i < nLabels; i++)
	{
		if (blobs[i].y < detected_line && (blobs[i].y + blobs[i].height) > detected_line)
		{ //blob que esta entre a linha da matricula
			if (blobs[i].area > max && (blobs[i].height - blobs[i].y) < (blobs[i].width - blobs[i].x))
			{
				//tenta encontrar o blob com maior area que esta na linha da matricula
				max = blobs[i].area;
				index = i;
				//printf("Label: %d | Perimetro: %d | Centro-MASSA: %d,%d | Area: %d | X: %d | Y : %d Width: %d | Height: %d\n", blobs[i].label, blobs[i].perimeter, blobs[i].xc, blobs[i].yc, blobs[i].area, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
			}
		}
	}

	//Extrai a matricula entrontrada para uma nova imagem
	if (index != -1)
		for (y = 0; y < original->height; y++)
			for (x = 0; x < original->width; x++)
			{
				pos = y * original->bytesperline + x * original->channels;

				if (y >= blobs[index].y && x >= blobs[index].x && y <= (blobs[index].y + blobs[index].height) && x <= (blobs[index].x + blobs[index].width))
					license->data[pos] = original->data[pos];
				else
					license->data[pos] = 255;
			}

	//Salva e executa a matricula recortada
	//vc_write_image("fase2_license.pgm", license);

	//Clean
	//free(prewitt);
	//free(filter);
	//free(clean);
	//
	return &(blobs[index]);
}

int vc_getBlob_Histogram(IVC* license, OVC* blob_character) {
	int x, y, pos,posk, minBlob_Y, minBlob_X, maxBlob_Y, maxBlob_X, max, yy, xx;

	//vc_write_image("license_teste.pgm", license);
	
	IVC* dilate = vc_image_new(license->width, license->height, 1, 255);
	IVC* temp = NULL, *temp2 = NULL;
	vc_binary_erode(license, dilate, 3);
	int blob = 2;
	int black = 0, white = 0;
	int* array = malloc(sizeof(blob_character[blob].width + 1));

	for (int i = 0; i < 6; i++)
	{
		minBlob_Y = blob_character[i].y;
		minBlob_X = blob_character[i].x;
		maxBlob_Y = blob_character[i].y + blob_character[i].height;
		maxBlob_X = blob_character[i].x + blob_character[i].width;
		
		if (i == blob)
		{
			temp = vc_image_new(blob_character[i].width, blob_character[i].height, 1, 255);
			temp2 = vc_image_new(blob_character[i].width, blob_character[i].height, 1, 255);
		}
			
		yy = 0;
		xx = 0;
		white = 0;
		black = 0;
		//Horizontal
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			max = 0;
			for (y = minBlob_Y; y < maxBlob_Y; y++)
			{
				//printf("y: %d, x: %d\n", y,x);
				pos = y * dilate->bytesperline + x * dilate->channels;

				if (dilate->data[pos] == 255)
					max++;
			}
			array[xx++] = max;
			//printf("max: %d\n", max);
		}
		int media1 = 0, media2 = 0, media3 = 0;
		int count1 = 0, count2 = 0, count3 = 0;
		xx = 0;
		int part = blob_character[i].height / 3;
		int extremes = 20 * blob_character[i].width / 100;
		int mid = 60 * blob_character[i].width / 100;
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			if (x < (minBlob_X + extremes))
			{
				media1 += array[xx++];
				count1++;
			}
			else if (x < (minBlob_X + extremes + mid))
			{
				media2 += array[xx++];
				count2++;
			}
			else {
				media3 += array[xx++];
				count3++;
			}
		}
		media1 /= count1;
		media2 /= count2;
		media3 /= count3;

		printf("media1: %d, media2: %d, media3: %d\n", media1, media2, media3);
	}
	printf("\n\n");
	return 0;
	
}

int vc_getBlob_Histogram_1(IVC* license, OVC* blob_character) {
	int x, y, pos, count = 0;
	int minBlob_X, minBlob_Y, maxBlob_X, maxBlob_Y;
	int width, height;
	//IVC* erode = vc_image_new(license->width, license->height, 1, 255);
	//vc_binary_erode(license, erode, 3);
	//vc_write_image("erode.pgm", erode);

	for (int i = 0; i < 6; i++)
	{
		count = 0;
		width = blob_character[i].width;
		height = blob_character[i].height;
		minBlob_Y = blob_character[i].y;
		minBlob_X = blob_character[i].x;
		maxBlob_Y = blob_character[i].y + blob_character[i].height;
		maxBlob_X = blob_character[i].x + blob_character[i].width;

		for (y = minBlob_Y; y < maxBlob_Y; y++)
		{
			pos = y * license->bytesperline + 2 * license->channels;
			if(license->data[pos] == 255)
				count++;
		}

		printf("count: %d", count);
		count = (count * 100) / height;
		printf("\tcount perc : %d\n", count);
	}
	return 0;
}

char vc_get_character(int horizontal, int invert_horizontal, int vertical, int max_pixels){

	int p_h = (horizontal * 100) / vertical;
	int p_i_h = (invert_horizontal * 100) / vertical;
	//int p_v = (vertical * 100) / max_pixels;
	int p_t = ((horizontal + invert_horizontal + vertical) * 100) / vertical;
	printf("H : %d , IH : %d \t|\t ", p_h,p_i_h);
	printf("Tot : %d \n", p_t);

	//Q
	if (p_h > 11 && p_h <= 20 && p_i_h >= 10 && p_i_h <= 16)
		return 'Q';
	//R
	else if (p_h >= 2 && p_h <= 8 && p_i_h >= 28 && p_i_h <= 34)
		return 'R';
	//U
	else if (p_h >= 5 && p_h <= 11 && p_i_h >= 11 && p_i_h <= 17)
		return 'U';
	
	//2
	else if (p_h >= 52 && p_h <= 58 && p_i_h >= 37 && p_i_h <= 43)
		return '2';
	//7
	else if (p_h >= 53 && p_h <= 58 && p_i_h >= 45 && p_i_h <= 51)
		return '7';
	//9
	else if (p_h >= 44 && p_h <= 50 && p_i_h >= 15 && p_i_h <= 21)
		return '9';
	//8
	else if (p_h >= 20 && p_h <= 26 && p_i_h >= 25 && p_i_h <= 31)
		return '8';
	//6
	else if (p_h >= 12 && p_h <= 18 && p_i_h >= 38 && p_i_h <= 44)
		return '6';
	//Not Found
	else
		return '-';
}

char vc_analise_blob_2(IVC* license, OVC blob_character)
{
	int x, y, pos, totBlack = 0, check = 0, max_pixels = blob_character.height * blob_character.width;
	int minBlob_X, minBlob_Y, maxBlob_X, maxBlob_Y;
	minBlob_Y = blob_character.y;
	minBlob_X = blob_character.x;
	maxBlob_Y = blob_character.y + blob_character.height;
	maxBlob_X = blob_character.x + blob_character.width;

	int meanH = 0, meanIH = 0, dvH = 0, dvIH = 0, countIH = 0, countH = 0;

	for (y = minBlob_Y; y < maxBlob_Y; y++)
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 255)
				meanH++;
		}
	meanH /= blob_character.height;

	for (y = minBlob_Y; y < maxBlob_Y; y++)
		for (x = maxBlob_X; x > minBlob_X; x--)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 255)
				meanIH++;
		}
	meanIH /= blob_character.height;

	//Horizontal
	for (y = minBlob_Y; y < maxBlob_Y; y++)
	{
		countH = 0;
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 255)
				countH++;
			else
				break;
		}
		dvH += pow(countH - meanH, 2);
	}
	dvH /= blob_character.height;
	dvH = sqrt(dvH);
	

	//Inverted Horizontal
	for (y = minBlob_Y; y < maxBlob_Y; y++)
	{
		countIH = 0;
		for (x = maxBlob_X; x > minBlob_X; x--)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 255)
				countIH++;
			else break;
		}
		dvIH += pow(countIH - meanIH, 2);
	}
	dvIH /= blob_character.height;
	dvIH = sqrt(dvIH);
			
	printf("H : %d , IH : %d\n",dvH,dvIH);
	return 'a';
}



char vc_analise_blob(IVC* license, OVC blob_character)
{
	int x, y, pos, countIH = 0, countH = 0, totBlack = 0, check = 0, max_pixels = blob_character.height * blob_character.width;
	int minBlob_X, minBlob_Y, maxBlob_X, maxBlob_Y;
	minBlob_Y = blob_character.y;
	minBlob_X = blob_character.x;
	maxBlob_Y = blob_character.y + blob_character.height;
	maxBlob_X = blob_character.x + blob_character.width;


	//Horizontal
	for (y = minBlob_Y; y < maxBlob_Y; y++)
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 0) 
				countH++;
			else 
				break;
		}

	//Inverted Horizontal
	for (y = minBlob_Y; y < maxBlob_Y; y++)
		for (x = maxBlob_X; x > minBlob_X; x--)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 0)
				countIH++;
			else
				break;
		}

	for (y = minBlob_Y; y < maxBlob_Y; y++)
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			pos = y * license->bytesperline + x * license->channels;
			if (license->data[pos] == 0)
				totBlack++;
		}

	return vc_get_character(countH,countIH, totBlack, max_pixels);
}

int vc_save_blob(IVC* license, OVC* blob_character){
	int x, y, tempX, tempY, pos, posO, countV = 0,countH = 0, blob = 2;
	int minBlob_X, minBlob_Y, maxBlob_X, maxBlob_Y;
	int width, height;
	IVC* saved_blob = vc_image_new(blob_character[blob].width, blob_character[blob].height, 1, 255);
	width = blob_character[blob].width;
	height = blob_character[blob].height;
	minBlob_Y = blob_character[blob].y;
	minBlob_X = blob_character[blob].x;
	maxBlob_Y = blob_character[blob].y + blob_character[blob].height;
	maxBlob_X = blob_character[blob].x + blob_character[blob].width;
	char characters[6];

	for (int i = 0; i < 6; i++)
	{
		printf("Blob %d --> ",i);
		characters[i] = vc_analise_blob_2(license, blob_character[i]);
		if (characters[i] == '-')
			return 0;
	}
	
	printf("license: %s\n", characters);
	getchar();

	for (y = minBlob_Y; y < maxBlob_Y; y++)
	{
		for (x = minBlob_X; x < maxBlob_X; x++)
		{
			posO = y * license->bytesperline + x * license->channels;
			pos = (y - minBlob_Y) * saved_blob->bytesperline + (x - minBlob_X) * saved_blob->channels;
			saved_blob->data[pos] = license->data[posO];
		}
	}
	/*
	printf("Width: %d , Height: %d \n\n", saved_blob->width, saved_blob->height);

	//V
	countV = 0;
	for (y = 0; y < saved_blob->height; y++)
	{
		pos = y * saved_blob->bytesperline + 1 * saved_blob->channels;
		if (saved_blob->data[pos] == 255)
			countV++;
	}
	printf("V - Blob: %d \t| countV: %d", blob, countV);
	countV = (countV * 100) / saved_blob->height;
	printf("\t| countV perc: %d\n", countV);


	//H
	countH = 0;
	for (x = 0; x < saved_blob->width; x++)
	{
		pos = (saved_blob->height - 2) * saved_blob->bytesperline + x * saved_blob->channels;
		if (saved_blob->data[pos] == 255)
			countH++;
	}
	printf("H - Blob: %d \t| countH: %d", blob, countH);
	countH = (countH * 100) / saved_blob->width;
	printf("\t| countH perc: %d\n", countH);


	//Save blob
	
	*/
	vc_write_image("blob_cut_3.pgm", saved_blob);
	return 0;
}

int vc_trab_fase3(IVC *original, IVC *result, IVC *license, OVC *blob, OVC *blob_character, int th)
{
	if (blob == NULL)
		return -1;

	//VARS
	int nLabels = 0;
	int x, y, pos, sum = 0, min, max, i = 0, j = 0;
	//
	OVC *blobs;
	IVC *prewitt, *binary, *bImg, * erode;

	//Inicialize
	prewitt = vc_image_new(original->width, original->height, 1, 255);
	binary = vc_image_new(original->width, original->height, 1, 255);
	erode = vc_image_new(original->width, original->height, 1, 255);
	bImg = vc_image_new(original->width, original->height, 1, 255);

	//Equalize
	//vc_gray_histogram_equalization(license, equalize);
	//vc_write_image("fase3_equalize.pgm", equalize);


	//Edge
	vc_trab_prewitt(license, prewitt);
	//vc_write_image("fase3_prewitt.pgm", prewitt); //Passa para outro nivel de cinzas

	vc_rgb_gray_to_binary(prewitt, binary, th);
	//vc_write_image("fase3_prewitt_2.pgm", binary);

	vc_binary_erode(binary,erode, 3);
	vc_write_image("fase3_erode.pgm", erode);

	//Get blobs
	blobs = vc_binary_blob_labelling(erode, bImg, &nLabels);
	vc_binary_blob_info(bImg, blobs, nLabels);

	//Salva e executa a imagem dos blobs
	//save("fase3_blobs.pgm", bImg);

	//Free
	//free(prewitt);
	//free(binary);
	//free(bImg);

	//Inicializate median array
	int median[255][3], count = 0, medianI;
	float mean = 0;

	//Primeiro filtro nos blobs
	for (i = 0; i < nLabels; i++)
	{
		if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40)
		{
			median[count][0] = i;
			median[count][1] = blobs[i].height;
			median[count++][2] = blobs[i].y;
			//printf("Label: %d | Perimetro: %d | Centro-MASSA: %d,%d | Area: %d | X: %d | Y : %d Width: %d | Height: %d\n", blobs[i].label, blobs[i].perimeter, blobs[i].xc, blobs[i].yc, blobs[i].area, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
		}
	}

	//Ordena o array por ordem ascendente
	for (i = 0; i < count; i++)
	{
		mean += median[i][2];
		for (j = 0; j < count; j++)
			if (median[j][1] > median[i][1])
			{
				int tmpI = median[i][0];
				int tmpHeight = median[i][1];
				int tmpY = median[i][2];
				median[i][0] = median[j][0];
				median[i][1] = median[j][1];
				median[i][2] = median[j][2];
				median[j][0] = tmpI;
				median[j][1] = tmpHeight;
				median[j][2] = tmpY;
			}
	}

	//Lista o array ordenado
	//printf("\n\n");
	//for (i = 0; i < count; i++)
	//	printf("Index : %d \t|\tHeight: %d \t|\tY : %d\n", median[i][0], median[i][1], median[i][2]);

	//Calcula a media e a mediana
	mean = mean / (float)count;
	medianI = count % 2 == 0 ? (count / 2) : ((count - 1) / 2);
	count = 0;

	//Calcula quantos blobs tem com o novo filtro
	//printf("\n\n");
	for (i = 0; i < nLabels; i++)
	{
		if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40 && (blobs[median[medianI][0]].height - 5) < blobs[i].height && (blobs[median[medianI][0]].height + 5) > blobs[i].height && (mean - 10) < blobs[i].y && (mean + 10) > blobs[i].y)
		{
			count++;
			//printf("Label: %d | Perimetro: %d | Centro-MASSA: %d,%d | Area: %d | X: %d | Y : %d Width: %d | Height: %d\n", blobs[i].label, blobs[i].perimeter, blobs[i].xc, blobs[i].yc, blobs[i].area, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
		}
	}

	//Verifica se o total de blobs é igual a 6
	if (count == 6)
	{
		

		count = 0;
		for (i = 0; i < nLabels; i++)
			if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40 && (blobs[median[medianI][0]].height - 5) < blobs[i].height && (blobs[median[medianI][0]].height + 5) > blobs[i].height && (mean - 10) < blobs[i].y && (mean + 10) > blobs[i].y)
				blob_character[count++] = blobs[i];

		vc_save_blob(erode, blob_character);
		//vc_getBlob_Histogram_1(binary, blob_character);
		
		//VARS AUX
		int bx = blob->x,					//x min
			by = blob->y,					//y min
			bMaxx = blob->x + blob->width,	//x max
			bMaxY = blob->y + blob->height; //y max

		//Pinta a matricula
		for (y = 0; y < original->height; y++)
			for (x = 0; x < original->width; x++)
			{
				pos = y * original->bytesperline + x * original->channels;

				if ((y == by && (x >= bx && x <= bMaxx)) ||	   //horizontal - top
					(x == bx && (y >= by && y <= bMaxY)) ||	   //veritcal   - right
					(y == bMaxY && (x >= bx && x <= bMaxx)) || //horizontal - down
					(x == bMaxx && (y >= by && y <= bMaxY)))   //vertical   - left
				{
					if (x == bx)
					{ //Vertical - left
						result->data[pos] = 255;
						result->data[pos + 1] = 0;
						result->data[pos + 2] = 0;
						//
						result->data[pos - 3] = 255;
						result->data[pos - 2] = 0;
						result->data[pos - 1] = 0;
					}
					if (y == by)
					{ //Horizontal - Up
						result->data[pos] = 255;
						result->data[pos + 1] = 0;
						result->data[pos + 2] = 0;
						//
						result->data[pos + result->bytesperline] = 255;
						result->data[pos + 1 + result->bytesperline] = 0;
						result->data[pos + 2 + result->bytesperline] = 0;
					}
					if (y == bMaxY)
					{ //Horizontal - Down
						result->data[pos] = 255;
						result->data[pos + 1] = 0;
						result->data[pos + 2] = 0;
						//
						result->data[pos - result->bytesperline] = 255;
						result->data[pos + 1 - result->bytesperline] = 0;
						result->data[pos + 2 - result->bytesperline] = 0;
					}
					if (x == bMaxx)
					{ //Vertical - Right
						result->data[pos] = 255;
						result->data[pos + 1] = 0;
						result->data[pos + 2] = 0;
						//
						result->data[pos + 3] = 255;
						result->data[pos + 4] = 0;
						result->data[pos + 5] = 0;
					}
				}
			}

		//Pinta os numeros e letras
		for (i = 0; i < nLabels; i++)
		{

			bx = blobs[i].x - (blobs[i].x * 0.01);						//x min
			by = blobs[i].y - (blobs[i].y * 0.01);						//y min
			bMaxx = blobs[i].x + blobs[i].width + (blobs[i].x * 0.01);	//x max
			bMaxY = blobs[i].y + blobs[i].height + (blobs[i].y * 0.01); //y max

			for (y = 0; y < original->height; y++)
				for (x = 0; x < original->width; x++)
				{
					pos = y * original->bytesperline + x * original->channels;

					if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40 && (blobs[median[medianI][0]].height - 5) < blobs[i].height && (blobs[median[medianI][0]].height + 5) > blobs[i].height && (mean - 10) < blobs[i].y && (mean + 10) > blobs[i].y)
						if ((y == by && (x >= bx && x <= bMaxx)) ||	   //horizontal - top
							(x == bx && (y >= by && y <= bMaxY)) ||	   //veritcal - right
							(y == bMaxY && (x >= bx && x <= bMaxx)) || //horizontal - down
							(x == bMaxx && (y >= by && y <= bMaxY)))   // vertical-left
						{
							if (x == bx)
							{ //Vertical - left
								result->data[pos] = 0;
								result->data[pos + 1] = 0;
								result->data[pos + 2] = 255;
								//
								result->data[pos - 3] = 0;
								result->data[pos - 2] = 0;
								result->data[pos - 1] = 255;
							}
							if (y == by)
							{ //Horizontal - Up
								result->data[pos] = 0;
								result->data[pos + 1] = 0;
								result->data[pos + 2] = 255;
								//
								result->data[pos + result->bytesperline] = 0;
								result->data[pos + 1 + result->bytesperline] = 0;
								result->data[pos + 2 + result->bytesperline] = 255;
							}
							if (y == bMaxY)
							{ //Horizontal - Down
								result->data[pos] = 0;
								result->data[pos + 1] = 0;
								result->data[pos + 2] = 255;
								//
								result->data[pos - result->bytesperline] = 0;
								result->data[pos + 1 - result->bytesperline] = 0;
								result->data[pos + 2 - result->bytesperline] = 255;
							}
							if (x == bMaxx)
							{ //Vertical - Right
								result->data[pos] = 0;
								result->data[pos + 1] = 0;
								result->data[pos + 2] = 255;
								//
								result->data[pos + 3] = 0;
								result->data[pos + 4] = 0;
								result->data[pos + 5] = 255;
							}
						}
				}
		}
	}
	else
		return 1;
	//
	return 0;
}

//Função prewitt para uma gama de cinzas
int vc_trab_prewitt(IVC *original, IVC *converted)
{
	int x, y, pos;
	int width = original->width, height = original->height;
	int yUp = 0, yDown = 0, xLeft = 0, xRight = 0;
	int gx, gy, edge;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			//
			yUp = MAX(y - 1, 0) * original->bytesperline;
			yDown = MIN(y + 1, height - 1) * original->bytesperline;
			xLeft = MAX(x - 1, 0);
			xRight = MAX(x + 1, width - 1);

			//Horizontal
			//| -1 0 +1 |
			//| -1 0 +1 |
			//| -1 0 +1 |
			gx = original->data[yUp + xRight] * -1;
			gx += original->data[y + xRight] * -1;
			gx += original->data[yDown + xRight] * -1;
			//
			gx += original->data[yUp + xLeft] * 1;
			gx += original->data[y + xLeft] * 1;
			gx += original->data[yDown + xLeft] * 1;
			//
			gx = gx / 3; //Magnitude 3

			//Verical
			//| -1 -1 -1 |
			//| 0  0   0 |
			//| +1 +1 +1 |
			gy = original->data[yUp + xLeft] * 1;
			gy += original->data[yUp + x] * 1;
			gy += original->data[yUp + xRight] * 1;
			//
			gy += original->data[yDown + xLeft] * -1;
			gy += original->data[yDown + x] * -1;
			gy += original->data[yDown + xRight] * -1;
			//
			gy = gy / 3; //Magnitude 3

			edge = (unsigned char)sqrt((float)(gx * gx + gy * gy));

			//Verifica se é menor que 0 ou maior que 255
			converted->data[pos] = edge < 0 ? 0 : edge > 255 ? 255 : edge;
		}

	return 0;
}

//Deteta a linha que contem o maior numero de transições entre preto e branco
int vc_trab_detect(IVC *original)
{

	//VARS
	int x, y, pos;
	int yMax = -1;
	int maxTrans = 0, aux = 0;
	int check = 0;
	check = original->data[0];

	IVC *temp = vc_image_new(original->width, original->height, 1, 255);

	//Calcula linha com maior numero de transições
	for (y = 0; y < original->height; y++)
	{
		aux = 0;
		check = original->data[(y * original->width) + 0];
		for (x = 50; x < (original->width - 50); x++)
		{
			//pos = y * original->bytesperline + x * original->channels;
			if (check != original->data[(y * original->width) + x])
			{
				check = original->data[(y * original->width) + x];
				aux++;
			}
		}
		if (aux > maxTrans && aux < 600)
		{
			maxTrans = aux;
			yMax = y;
		}
	}

	//Pinta na imagem uma linha no Y encontrado
	for (y = 0; y < original->height; y++)
	{
		for (x = 1; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (y == yMax)
				temp->data[pos] = 127;
			else
				temp->data[pos] = original->data[pos];
		}
	}
	//save("fase1_detect.pgm", temp);
	//vc_write_image(aux, temp);
	return yMax;
}



int vc_convert_bgr_rgb(IVC* original) {
	int x, y, pos;
	int temp;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;
			temp = original->data[pos];
			original->data[pos] = original->data[pos + 2];
			original->data[pos + 2] = (unsigned char)temp;
		}
	}
}

//Limpa uma percentagem da imagem baseado no Y onde foi detectada a matricula
int vc_trab_clean(IVC* original, IVC* converted, int license_line, float percent)
{
	int x, y, pos;

	if (percent <= 0)
		return 0;

	//Calcula as percentagens
	percent /= 100;
	int aux = original->height * percent;
	float min = license_line - aux >= 0 ? license_line - aux : 0,
		max = license_line + aux <= original->height ? license_line + aux : original->height;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			pos = y * original->bytesperline + x * original->channels;
			if (y <= min || y >= max)
				converted->data[pos] = 0;
			else
				converted->data[pos] = original->data[pos] == 255 ? 255 : original->data[pos];
		}

	return 1;
}

int vc_binary_dilate_x(IVC* original, IVC* converted, int kernel) {
	long int pos, posk;
	float meanAux;
	int channels = original->channels, bytesperline = original->bytesperline;
	int offset = (kernel - 1) / 2, offset_dyn;
	int x, kx, y, ky;
	int width = original->width, height = original->height;
	float threshold;
	int auxCol, auxLine;
	int max = 0;

	if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL)) return 0;
	if ((original->width != converted->width) || (original->height != converted->height) || (original->channels != converted->channels)) return 0;
	if (channels != 1) return 0;


	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++)
		{
			offset_dyn = 0;
			pos = y * original->bytesperline + x * original->channels;
			max = 0;
			//
			converted->data[pos] = 255;
			for (ky = -offset; ky <= offset; ky++) {
				for (kx = -offset_dyn; kx <= offset_dyn; kx++) {
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;
						//
						if (original->data[posk] > max)
							max = original->data[posk];
					}
				}
				if (ky < 0) offset_dyn++;
				else offset_dyn--;
			}
			converted->data[pos] = max;
		}

	return 1;
}

int vc_increase_contraste(IVC *original, IVC *converted, int perc) {
	int pos, x, y, count= 0;

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++) {
			pos = y * original->bytesperline + x * original->channels;
			count += original->data[pos];
		}

	count /= original->height * original->width;
	int percApply = 20; //= count * (perc / 100);

	for (y = 0; y < original->height; y++)
		for (x = 0; x < original->width; x++) {
			pos = y * original->bytesperline + x * original->channels;
			if (original->data[pos] >= 127)
				converted->data[pos] = original->data[pos] + percApply;
			else
				converted->data[pos] = original->data[pos] - percApply;
		}

	return 0;
}

int vc_rgb_negative(IVC* original, IVC* converted)
{
	int pos, x, y;
	int max = 255;

	for (x = 0; x < original->width; x++)
	{
		for (y = 0; y < original->height; y++)
		{
			pos = y * original->bytesperline + x * original->channels;

			converted->data[pos] = max - original->data[pos];
			converted->data[pos + 1] = max - original->data[pos + 1];
			converted->data[pos + 2] = max - original->data[pos + 2];
		}
	}

	return 0;
}
#pragma endregion
