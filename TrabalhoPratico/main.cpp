#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#pragma warning(disable : 4996)

using namespace std;

extern "C"
{
	#include "vc.h"	
}

char* vc_trab_fase3(IVC* original, IVC* result, IVC* license, OVC* blob, int th, cv::Mat db[8]);
void vc_timer(void);

int main(void)
{
	// V�deo
	char videofile[60] = "video1.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat db[8];
	db[0] = cv::imread("db\\2.pgm", 0);
	db[1] = cv::imread("db\\6.pgm", 0);
	db[2] = cv::imread("db\\7.pgm", 0);
	db[3] = cv::imread("db\\8.pgm", 0);
	db[4] = cv::imread("db\\9.pgm", 0);
	db[5] = cv::imread("db\\Q.pgm", 0);
	db[6] = cv::imread("db\\R.pgm", 0);
	db[7] = cv::imread("db\\U.pgm", 0);

	IVC* img, * license, * result, * gray, * contraste;
	OVC* blob;
	int detected_line = -1, count = 0, check = 0;
	int nMatriculas = 0;
	char *licenses[100], *newLicense;

	cv::Mat frame;
	while (key != 'q')
	{
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);


		/***********************************************************************************/
		/*****************************		My Own Code		********************************/
		/***********************************************************************************/

		// Cria uma nova imagem IVC
		img = vc_image_new(video.width, video.height, 3, 255);

		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(img->data, frame.data, (video.width * video.height * 3));

		//Fase 1 - Detectar o Y em que esta a matricula
		//Fase 2 - Detectar o maior Blob que esta na linha onde foi encontrada a matricula
		//Fase 3 - Detectar os numeros/letras e carregar na nova imagem
		gray = vc_image_new(img->width, img->height, 1, img->levels);               //imagem rgb para gray
		contraste = vc_image_new(img->width, img->height, 1, img->levels);               //imagem rgb para gray
		license = vc_image_new(img->width, img->height, 1, img->levels);            //imagem com a matricula recortada
		result = vc_image_new(img->width, img->height, img->channels, img->levels); //imagem final com a matricula aplicada
		memcpy(result->data, img->data, img->bytesperline * img->height);           //Copia dos dados da imagem original
		check = 0;

		//FASE 1
		vc_bgr_gray(img, gray);       //RGB para Gray

		detected_line = vc_trab_fase1(gray);
		if (detected_line != -1)
		{
			//FASE 2
			blob = vc_trab_fase2(gray, license, detected_line);
			//FASE 3
			if (detected_line != -1 && blob != NULL)
			{
				//1 chance
				newLicense = vc_trab_fase3(img, result, license, blob, 140, db);//aplica a fase 3
				if (strcmp(newLicense,"-") == 0) {
					//2 chance
					vc_increase_contraste(license, license, 128); //Aplica um contraste a imagem
					newLicense = vc_trab_fase3(img, result, license, blob, 165, db);//Tenta novamente aplicar a fase 3
				}

				//Verifica se retornou uma matricula
				if (strcmp(newLicense, "-") != 0) {//verifica se retornou uma matricula

					//Verifica se ja existe esta matricula
					for (int c = 0;c < nMatriculas;c++) {
						if (strcmp(licenses[c], newLicense) == 0) {
							check = 1;
						}	
					}

					//Insere na lista de matriculas caso ainda não exista
					if (!check) {// 1 = ja existe, 0 = não existe
						licenses[nMatriculas] = (char*)malloc((6 + 1) * sizeof(char *));
						strcpy(licenses[nMatriculas++], (char*)newLicense);
					}
							
					//Numero de matriculas reconhecidas
					count++;
				}
			}				
		}

		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, result->data, (video.width* (video.height * 3)));

		/***********************************************************************************/
		/***************************	END Own Code	************************************/
		/***********************************************************************************/


		/* Exemplo de inser��o texto na frame */
		int b = 0;
		int g = 0;
		int r = 255;
		float size = 0.7;
		int font = 4;
		/*
		FONT_HERSHEY_SIMPLEX = 0,
		FONT_HERSHEY_PLAIN = 1,
		FONT_HERSHEY_DUPLEX = 2,
		FONT_HERSHEY_COMPLEX = 3,
		FONT_HERSHEY_TRIPLEX = 4,
		FONT_HERSHEY_COMPLEX_SMALL = 5,
		FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
		FONT_HERSHEY_SCRIPT_COMPLEX = 7,
		FONT_ITALIC = 16
		*/
		char *aux = (nMatriculas > 0) ? (char *)licenses[nMatriculas-1] : (char*)"";
		str = std::string("MATRICULA: ").append(aux);
		cv::putText(frame, str, cv::Point(20, video.height - 75), font, size, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, video.height - 75), font, size, cv::Scalar(b, g, r), 1);
		str = std::string("FRAME: ").append(std::to_string(video.nframe) + "/" + std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, video.height - 50), font, size, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, video.height - 50), font, size, cv::Scalar(b, g, r), 1);
		str = std::string("N de matriculas: ").append(std::to_string(nMatriculas));
		cv::putText(frame, str, cv::Point(20, video.height - 25), font, size, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, video.height - 25), font, size, cv::Scalar(b, g, r), 1);

		/* Exibe a frame */
		cv::imshow("VC", frame);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);

	}

	printf("\nMatriculas Detectadas: \n");
	for (int i = 0; i < nMatriculas;i++) {
		printf("\t%s\n",licenses[i]);
	}

	/* Para o timer e exibe o tempo decorrido */
	printf("\n");
	vc_timer();


	/* Fecha a janela */
	cv::destroyWindow("VC");

	/* Fecha o ficheiro de v�deo */
	capture.release();


	return 0;
}
