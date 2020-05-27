#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

using namespace std;

extern "C"
{
	#include "vc.h"
}


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}


int main(void)
{
	// V�deo
	char videofile[60] = "video1.mp4";
	int nMatriculas = 0;
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

	IVC* img, * license, * result, * gray, * contraste;
	OVC* blob, * blob_characters;
	int detected_line = -1;

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


		//Convert to RGB
		vc_convert_bgr_rgb(img);

		printf("Frame: %d\n", video.nframe);


		//Fase 1 - Detectar o Y em que esta a matricula
		//Fase 2 - Detectar o maior Blob que esta na linha onde foi encontrada a matricula
		//Fase 3 - Detectar os numeros/letras e carregar na nova imagem
		gray = vc_image_new(img->width, img->height, 1, img->levels);               //imagem rgb para gray
		contraste = vc_image_new(img->width, img->height, 1, img->levels);               //imagem rgb para gray
		license = vc_image_new(img->width, img->height, 1, img->levels);            //imagem com a matricula recortada
		result = vc_image_new(img->width, img->height, img->channels, img->levels); //imagem final com a matricula aplicada
		memcpy(result->data, img->data, img->bytesperline * img->height);           //Copia dos dados da imagem original
		blob_characters = (OVC*)calloc((6), sizeof(OVC));


		//FASE 1
		vc_rgb_gray(img, gray);       //RGB para Gray
		//vc_write_image((char*)"gray.pgm", gray);
		//vc_increase_contraste(gray, contraste, 10);
		//vc_write_image((char *)"constraste.pgm", contraste);

		if (video.nframe > 400)
		{
			detected_line = vc_trab_fase1(gray);
			printf("Fase 1 concluida\n");
			//
			if (detected_line != -1)
			{
				//FASE 2
				blob = vc_trab_fase2(gray, license, detected_line);

				//FASE 3
				if (detected_line != -1)
				{
					if (vc_trab_fase3(img, result, license, blob, blob_characters, 140)) {
						vc_trab_fase3(img, result, license, blob, blob_characters, 123);
					}
					printf("Fase 3 concluida\n\n");
					//vc_write_image((char *)"teste.ppm", result);
				}				
			}
			else
				printf("Frame %d: A matricula não foi detectada.\n", video.nframe);
		}

		//Convert to BGR
		vc_convert_bgr_rgb(result);

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

		str = std::string("MATRICULA: ").append("77-QR-98");
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

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC");

	/* Fecha o ficheiro de v�deo */
	capture.release();


	return 0;
}
