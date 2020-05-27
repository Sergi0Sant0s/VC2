/*
Objetivo:
  Reconhecer e pintar numeros/letras e placa de uma matricula

Trabalho Realizado por:
  a17616  - José Rodrigues
  a12314  - Sérgio Santos
  a4561   - Ulisses Ferreira
*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
//
#include "lib/vc.h"

int main(void)
{
  //Portuguese
  setlocale(LC_ALL, "");

  const char *resourses = "resourses/";

  IVC *img, *license, *result, *gray;
  OVC *blob;
  int detected_line = -1;

  //Imagem de entrada
  img = vc_read_image(conc(resourses, "img1.ppm"));

  if (img == NULL)
  {
    printf("Error -> vc_img1_new():\n\tOut of memory!\n");
    return 0;
  }

  gray = vc_image_new(img->width, img->height, 1, img->levels);               //imagem rgb para gray
  license = vc_image_new(img->width, img->height, 1, img->levels);            //imagem com a matricula recortada
  result = vc_image_new(img->width, img->height, img->channels, img->levels); //imagem final com a matricula aplicada
  memcpy(result->data, img->data, img->bytesperline * img->height);           //Copia dos dados da imagem original

  //Fase 1 - Detectar o Y em que esta a matricula
  //Fase 2 - Detectar o maior Blob que esta na linha onde foi encontrada a matricula
  //Fase 3 - Detectar os numeros/letras e carregar na nova imagem

  //FASE 1
  vc_rgb_gray(img, gray);       //RGB para Gray
  save("fase1_gray.pgm", gray); //Salva e executa a imagem
  detected_line = vc_trab_fase1(gray);
  //
  if (detected_line != -1)
  {
    //FASE 2
    blob = vc_trab_fase2(gray, license, detected_line);

    //FASE 3
    vc_trab_fase3(img, result, license, blob) == 0 ? save("final_8.ppm", result) : printf("A matricula não foi detectada.\n");
  }
  else
    printf("A matricula não foi detectada.\n");

  //FREE
  vc_image_free(img);
  vc_image_free(result);
  vc_image_free(license);

  return 0;
}
*/