#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2\highgui\highgui.hpp>

extern "C"
{
#include "vc.h"
}

#define HEIGHT 40
#define WIDTH 20

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

char GetChar(int checkValue) {
	switch (checkValue)
	{
	case 0: return '2';
	case 1: return '6';
	case 2: return '7';
	case 3: return '8';
	case 4: return '9';
	case 5: return 'Q';
	case 6: return 'R';
	case 7: return 'U';
	}
	return '-';
}

float correlacao(cv::Mat image1, cv::Mat image2)
{
	/*
	TM_SQDIFF        = 0,
    TM_SQDIFF_NORMED = 1,
    TM_CCORR         = 2,
    TM_CCORR_NORMED  = 3,
    TM_CCOEFF        = 4,
    TM_CCOEFF_NORMED = 5,
	*/

	cv::Mat corr;
	cv::matchTemplate(image1, image2, corr, 3);
	return corr.at<float>(0, 0);  // corr only has one pixel
}

char* GetLicense(IVC* img[6], cv::Mat db[8]) {
	cv::Mat opencvImg[6];
	char license[6] = {'-','-', '-', '-', '-', '-'}, aux2;
	float corr, aux;
	int index, x , y, pos, i;
	
	
	//Inicia as variaveis com o tamanho por defeito
	for (i = 0;i < 6;i++) {
		opencvImg[i] = cv::Mat::zeros(cv::Size(WIDTH, HEIGHT), CV_64FC1);
		//repair blobs
		vc_binary_open(img[i], img[i], 3);
	}
	
	//Corre cada blob
	for (i = 0;i < 6;i++) {
		cv::Mat temp = cv::Mat(img[i]->height, img[i]->width,0);

		//Copia dos dados do carater
		memcpy(temp.data, img[i]->data, (img[i]->width * img[i]->height));


		//Resize
		resize(temp, opencvImg[i], cv::Size(WIDTH, HEIGHT));

		for(y = 0;y < opencvImg[i].rows;y++ )
			for (x = 0; x < opencvImg[i].cols; x++) {
				pos = y * (opencvImg[i].cols * 1) + x;
				opencvImg[i].data[pos] = opencvImg[i].data[pos] > 127 ? 255 : 0;
			}

		//Correlaçao
		index = -1;
		corr = 0;
		aux = 0;
		//
		for(int c = 0;c < 8;c++) {
			aux = correlacao(opencvImg[i], db[c]); //calcula a correlação das duas imagens
			if (aux > corr) {
				corr = aux;
				index = c;
			}
		}
		if (corr > 0.86) { //so aceita correlações acima de 0.86(0.0 - 1.0)
			aux2 = GetChar(index);
			if (aux2 != '-')
				license[i] = aux2;
			else return (char *)"-";
		}
		else break;
	}

	//Verifica se todos os carateres estão preenchidos
	for (i = 0;i < 6;i++) {
		if (license[i] == '-')
			return (char*)"-";
	}

	//Retorna a matricula
	return license;
}

char* vc_trab_fase3(IVC* original, IVC* result, IVC* license, OVC* blob, int th, cv::Mat db[8])
{
	//VARS
	int nLabels = 0;
	int x, y, pos, sum = 0, min, max, i = 0, j = 0;
	//
	OVC* blobs;
	IVC* prewitt, * binary, * bImg, * close;
	IVC* characters[6];
	char* tempLicense;

	//Inicialize
	prewitt = vc_image_new(original->width, original->height, 1, 255);
	binary = vc_image_new(original->width, original->height, 1, 255);
	close = vc_image_new(original->width, original->height, 1, 255);
	bImg = vc_image_new(original->width, original->height, 1, 255);

	//vc_write_image((char*)"fase3_license.pgm", license);

	//Edge
	vc_trab_prewitt(license, prewitt);
	//vc_write_image((char *)"fase3_prewitt.pgm", prewitt); //Passa para outro nivel de cinzas

	vc_rgb_gray_to_binary(prewitt, binary, th);
	//vc_write_image((char *)"fase3_prewitt_2.pgm", binary);

	vc_binary_close(binary, close, 3);
	//vc_write_image((char *)"fase3_erode.pgm", erode);

	//Get blobs
	blobs = vc_binary_blob_labelling(close, bImg, &nLabels);
	vc_binary_blob_info(bImg, blobs, nLabels);

	//Inicializate median array
	int median[255][3], count = 0, medianI;
	float mean = 0;

	//Primeiro filtro nos blobs
	for (i = 0; i < nLabels; i++)
		if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40)
		{
			median[count][0] = i;
			median[count][1] = blobs[i].height;
			median[count++][2] = blobs[i].y;
		}

	//Ordena o array por ordem ascendente
	int tmpI, tmpHeight, tmpY;
	for (i = 0; i < count; i++)
	{
		mean += median[i][2];
		for (j = 0; j < count; j++)
			if (median[j][1] > median[i][1])
			{
				tmpI = median[i][0];
				tmpHeight = median[i][1];
				tmpY = median[i][2];
				median[i][0] = median[j][0];
				median[i][1] = median[j][1];
				median[i][2] = median[j][2];
				median[j][0] = tmpI;
				median[j][1] = tmpHeight;
				median[j][2] = tmpY;
			}
	}

	//Calcula a media e a mediana
	mean = mean / (float)count;
	medianI = count % 2 == 0 ? (count / 2) : ((count - 1) / 2);
	count = 0;

	//Calcula quantos blobs tem com o novo filtro
	for (i = 0; i < nLabels; i++)
		if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40 && (blobs[median[medianI][0]].height - 10) < blobs[i].height && (blobs[median[medianI][0]].height + 10) > blobs[i].height && (mean - 10) < blobs[i].y && (mean + 10) > blobs[i].y)
			count++;

	//Verifica se o total de blobs é igual a 6
	if (count == 6)
	{

		int pos, pos0;
		count = 0;
		OVC tempBlobs[6], temp;

		//Armazena em 1 array todos os blobs que são carateres
		for (int i = 0;i < nLabels;i++) 
			if (blobs[i].height > blobs[i].width && blobs[i].width < blob->width * 0.15 && blobs[i].height > blob->height * 0.40 && (blobs[median[medianI][0]].height - 10) < blobs[i].height && (blobs[median[medianI][0]].height + 10) > blobs[i].height && (mean - 10) < blobs[i].y && (mean + 10) > blobs[i].y)
				tempBlobs[count++] = blobs[i];
			

		//Ordena os blobs pelo X menor
		for (i = 0; i < 6 - 1; i++)
			for (j = 0; j < (6 - 1 - i); j++)
				if (tempBlobs[j].x > tempBlobs[j + 1].x)
				{
					temp = tempBlobs[j];
					tempBlobs[j] = tempBlobs[j + 1];
					tempBlobs[j + 1] = temp;
				}

		//Copia a parte dos blobs para imagens isoladas
		for (int t = 0;t < 6;t++) {
			characters[t] = vc_image_new(tempBlobs[t].width, tempBlobs[t].height, 1, 255);
			for (int y = tempBlobs[t].y;y < tempBlobs[t].y + tempBlobs[t].height;y++)
				for (int x = tempBlobs[t].x;x < tempBlobs[t].x + tempBlobs[t].width;x++)
				{
					pos = (y - tempBlobs[t].y) * characters[t]->bytesperline + (x - tempBlobs[t].x);
					pos0 = y * close->bytesperline + x;
					characters[t]->data[pos] = close->data[pos0];
				}
		}
			
		//Detecta os carateres
		tempLicense = GetLicense(characters, db);

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
		return (char*)"-";
	//
	return tempLicense;
}