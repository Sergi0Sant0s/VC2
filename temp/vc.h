/*
Objetivo:
  Reconhecer e pintar numeros/letras e placa de uma matricula

Trabalho Realizado por:
  a17616  - José Rodrigues
  a12314  - Sérgio Santos
  a4561   - Ulisses Ferreira
*/

#define VC_DEBUG

#pragma region Structs

typedef struct
{
	unsigned char *data;
	int width, height;
	int channels;	  // Binario/Cinzentos=1; RGB=3
	int levels;		  // Binario=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline; // width * channels
} IVC;

typedef struct
{
	int x, y, width, height; // Caixa Delimitadora (Bounding Box)
	int area;				 // area
	int xc, yc;				 // Centro-de-massa
	int perimeter;			 // Peremetro
	int label;				 // Etiqueta
} OVC;

typedef struct
{
	char* caracter;
	int media1;
	int media2;
	int media3;
} Character;



#pragma endregion

#pragma region ALOCAR E LIBERTAR UMA IMAGEM

IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

#pragma endregion

#pragma region LEITURA E ESCRITA DE IMAGENS(PBM, PGM E PPM)

IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

#pragma endregion

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    Minhas Funções
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma region Save and Run

char *conc(const char *first, char *second);
void execute(char *filepath);
void save(char *filename, IVC *image);
void save_run(char *filename, IVC *image);

#pragma endregion

#pragma region RGB to GRAY

int vc_rgb_gray(IVC *original, IVC *converted);

#pragma endregion

#pragma region Binary

float vc_media(IVC *original);
int vc_rgb_gray_to_binary_global_mean(IVC *original, IVC *converted);
int vc_rgb_gray_to_binary(IVC *original, IVC *converted, int threshold);
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int c);

#pragma endregion

#pragma region Normal dilate + erode

int vc_binary_dilate(IVC *original, IVC *converted, int kernel);
int vc_binary_erode(IVC *original, IVC *converted, int kernel);

#pragma endregion

#pragma region open + close

int vc_binary_open(IVC *original, IVC *converted, int kernel);
int vc_binary_close(IVC *original, IVC *converted, int kernel);

#pragma endregion

#pragma region Labeling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

#pragma endregion

#pragma region Edge

int vc_binary_edge_prewitt(IVC *src, IVC *dst, float th);
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);
int vc_binary_edge_sobel(IVC* src, IVC* dst, float th);

#pragma endregion

#pragma region Histogram

int vc_rgb_histogram_equalization(IVC* original, IVC* converted);
int vc_gray_histogram_equalization(IVC* original, IVC* converted);
int vc_rgb_histogram_show(IVC* original, IVC* converted);
IVC* vc_gray_histogram_show(IVC* original);
float* vc_histogram_array(IVC* original);

#pragma endregion

#pragma region Filtros

int vc_gray_lowpass_mean_filter(IVC *original, IVC *converted);

#pragma endregion

#pragma region Trabalho
int vc_rgb_negative(IVC* original, IVC* converted);

//Metodos auxiliares
int vc_trab_detect(IVC *original);
int vc_trab_clean(IVC* original, IVC* converted, int license_line, float percent);
int vc_trab_prewitt(IVC *original, IVC *converted);
int vc_convert_bgr_rgb(IVC* original); 
int vc_binary_dilate_x(IVC* original, IVC* converted, int kernel);
int vc_increase_contraste(IVC* original, IVC* converted, int perc);

//Fases
int vc_trab_fase1(IVC *original);
OVC *vc_trab_fase2(IVC *original, IVC *license, int detected_line);
int vc_trab_fase3(IVC *original, IVC *result, IVC *license, OVC *blob, OVC* blob_character, int th);

#pragma endregion
