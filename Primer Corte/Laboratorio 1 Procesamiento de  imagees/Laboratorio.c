#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#pragma pack(1) // Asegura el alineamiento de 1 byte

// Estructura de cabecera
typedef struct {
    char bm[2];   // Tipo de fichero "BM"
    int fileSize;        // Tamaño del archivo
    short reserved1;     // Reservado1
    short reserved2;     // Reservado2
    int dataOffset;      // Inicio de los datos de la imagen
    int headerSize;       // Tamaño de la cabecera del bitmap
    int imageWidth;       // Anchura (píxels)
    int imageHeight;      // Altura (píxels)
    short planes;         // Número de planos
    short bitsPerPixel;   // Tamaño de cada punto
    int compression;      // Compresión (0=no comprimido)
    int imageSize;        // Tamaño de la imagen
    int xPixelsPerMeter;  // Resolución horizontal
    int yPixelsPerMeter;  // Resolución vertical
    int colorsUsed;       // Tamaño de la tabla de color
    int importantColors;  // (Contador de colores importantes
} BMPHeader;

// Estructura para pixeles de imagenes BMP
typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;

// Definición de los kernels Sobel
int sobel_x[3][3] = {{-1, 0, 1},
                     {-2, 0, 2},
                     {-1, 0, 1}};

int sobel_y[3][3] = {{-1, -2, -1},
                     { 0,  0,  0},
                     { 1,  2,  1}};

const char* outputFilePathGS = "GrayScaleConversion1.bmp";
const char* outputFilePathCV = "convolution2.bmp";
const char* outputFilePathCS = "convolutionSobel.bmp";


// Functions to process the image
void grayscale(Pixel* pixels, int width, int height, BMPHeader head);
void convolution(Pixel* pixels, int width, int height, int** kernel, BMPHeader head);
void sobelOperator(Pixel *pixels, int width, int height, BMPHeader head);
void safeImageBMP(Pixel *imagePixels, const char* outputFilePath, int width, int height, BMPHeader head);
int** createMatrix();
void printKernel(int** matrix);
int main() {
    FILE *bmpFile;
    BMPHeader header;
    Pixel *pixels;

    int width, height, option;
    int** kernel;

    // Abrir el archivo BMP en modo binario de lectura
    bmpFile = fopen("C:/Users/HABI/Desktop/SistemasEmbebidos/GrayScaleConversion1.bmp", "rb");
    if (bmpFile == NULL) {
        printf("No se pudo abrir el archivo BMP.\n");
        return 1;
    }
    // Leer la cabecera del archivo BMP
    fread(&header, sizeof(BMPHeader), 1, bmpFile);

    // Verifica que los primeros dos bytes sean BM
    if (header.bm[0] != 'B' || header.bm[1] != 'M') {
        printf("El archivo no es un BMP válido.\n");
        fclose(bmpFile);
        return 1;
    }

    // Obtener el tamaño de la imagen
    width = header.imageWidth;
    height = header.imageHeight;

    // Separa memoria para la matriz
    pixels = (Pixel *)malloc(width * height * sizeof(Pixel)); //asignacion dinamica de memoria
    if (pixels == NULL) {
        printf("Error al asignar memoria para la matriz de píxeles.\n");
        fclose(bmpFile);
        return 1;
    }

    // Leer los píxeles del archivo BMP y los copia en la matriz
    fseek(bmpFile, header.dataOffset, SEEK_SET); // Mueve el puntero al inicio de la imagen
    fread(pixels, sizeof(Pixel), width * height, bmpFile);
    // se pueden leer los datos de la matriz con pixels[i * width + j] donde i es la fila y j es la columna



    // Cerrar el archivo BMP
    fclose(bmpFile);

    printf("Menu:\n");
    printf("1. Escala de grises\n");
    printf("2. Convolucion\n");
    printf("3. Operador Sobel\n");
    printf("Elija una opcion\n");
    scanf_s("%d", &option);

    switch (option) {
        case 1:
            grayscale(pixels, width, height, header);
            break;
        case 2:
            kernel = createMatrix();
            printKernel(kernel);
            convolution(pixels, width, height, kernel, header);
            break;
        case 3:
            sobelOperator(pixels, width, height, header);
            break;
        default:
            printf("Opción no válida.\n");
            free(pixels);
            return 1;
    }

    return 0;
}

void grayscale(Pixel *pixels, int width, int height, BMPHeader head) {
    unsigned char grayValue;

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {

            Pixel *pixelGrayScale = &pixels[ j * width + i];

            grayValue = (unsigned char) ((pixelGrayScale->red * 0.299) + (pixelGrayScale->green * 0.587) + (pixelGrayScale->blue * 0.114));
            pixelGrayScale->red = grayValue;
            pixelGrayScale->green = grayValue;
            pixelGrayScale->blue = grayValue;
        }
    }
    safeImageBMP(pixels, outputFilePathGS, width, height, head);
}

void convolution(Pixel *pixels, int width, int height, int** kernel, BMPHeader head) {
    Pixel *pixelAux =  (Pixel *)malloc(width * height * sizeof(Pixel));
    int sumaRed, sumaGreen, sumaBlue, index, indexAux;
    for (int j = 1; j < height - 1; ++j) {
        for (int i = 1; i < width - 1; ++i) {
            sumaRed = 0;
            sumaGreen = 0;
            sumaBlue = 0;
            for (int u = 0; u < 3 ; ++u) {
                for (int v = 0; v < 3; ++v) {
                    index =(j - u + 1) * width + ( i - v + 1);
                    sumaRed += pixels[index].red * kernel[u][v];
                    sumaGreen += pixels[index].green * kernel[u][v];
                    sumaBlue += pixels[index].blue * kernel[u][v];
                }
            }
            indexAux = j * width + i;
            pixelAux[indexAux].red = (unsigned) sumaRed/9;
            pixelAux[indexAux].green = (unsigned) sumaGreen/9;
            pixelAux[indexAux].blue = (unsigned) sumaBlue/9;
        }
    }
    for (int z = 0; z < width * height; ++z) {
        pixels[z].red = pixelAux[z].red;
        pixels[z].green = pixelAux[z].green;
        pixels[z].blue = pixelAux[z].blue;
    }
    free(pixelAux);
    safeImageBMP(pixels, outputFilePathCV, width, height, head);
}

void safeImageBMP(Pixel *imagePixels, const char* outputFilePath, int width, int height, BMPHeader head){
    int totalColors = 0;
    FILE* outputFile = fopen(outputFilePath, "wb");
    if (!outputFile) {
        printf("Error al abrir el archivo de salida.\n");
        free(imagePixels);
    }

    fwrite("BM", 1, 2, outputFile);
    head.fileSize = 54 + (width * height * 3);
    fwrite(&head.fileSize, sizeof(int), 1, outputFile);
    head.reserved1= 0;
    fwrite(&head.reserved1, sizeof(int), 1, outputFile);
    head.dataOffset  = 54;
    fwrite(&head.dataOffset, sizeof(int), 1, outputFile);
    head.headerSize = 40;
    fwrite(&head.headerSize, sizeof(int), 1, outputFile);
    fwrite(&width, sizeof(int), 1, outputFile);
    fwrite(&height, sizeof(int), 1, outputFile);
    head.planes = 1;
    fwrite(&head.planes, sizeof(short), 1, outputFile);
    head.bitsPerPixel = 24;
    fwrite(&head.bitsPerPixel, sizeof(short), 1, outputFile);
    head.compression = 0;
    fwrite(&head.compression, sizeof(int), 1, outputFile);
    head.imageSize = width * height * 3;
    fwrite(&head.imageSize, sizeof(int), 1, outputFile);
    head.xPixelsPerMeter = 0;
    head.yPixelsPerMeter = 0;
    fwrite(&head.xPixelsPerMeter, sizeof(int), 1, outputFile);
    fwrite(&head.yPixelsPerMeter, sizeof(int), 1, outputFile);
    fwrite(&totalColors, sizeof(int), 1, outputFile);
    head.importantColors = 0;
    fwrite(&head.importantColors, sizeof(int), 1, outputFile);

    fseek(outputFile, 54, SEEK_SET);
    fwrite(imagePixels, sizeof(Pixel), width * height, outputFile);
    fclose(outputFile);

    free(imagePixels);
    printf("Imagen convertida y guardada '%s'\n", outputFilePath);
}
int** createMatrix(){
    printf("Por favor ingrese la matriz Kernel que desea usar\n");
    int** matrixKernel = (int**) malloc(3 * sizeof(int**));
    for (int i = 0; i < 3; ++i) {
        matrixKernel[i] = (int*) malloc(3 * sizeof (int));
        for (int j = 0; j < 3; ++j) {
            scanf_s("%d", &matrixKernel[i][j]);
        }
    }
    return matrixKernel;
}
void printKernel(int** matrix){
    printf("Usted ingreso el siguiente Kernel\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

}

void convolutionSobel(Pixel *pixels, int width, int height, int kernel[3][3], Pixel *output, BMPHeader head) {
    int index, indexAux;
    for (int j = 1; j < height - 1; ++j) {
        for (int i = 1; i < width - 1; ++i) {
            int sumaRed = 0;
            int sumaGreen = 0;
            int sumaBlue = 0;
            for (int u = 0; u < 3; ++u) {
                for (int v = 0; v < 3; ++v) {
                    index = (j - u + 1) * width + (i - v + 1);
                    sumaRed += pixels[index].red * kernel[u][v];
                    sumaGreen += pixels[index].green * kernel[u][v];
                    sumaBlue += pixels[index].blue * kernel[u][v];
                }
            }
            indexAux = j * width + i;
            output[indexAux].red = (unsigned char)(sumaRed / 9);
            output[indexAux].green = (unsigned char)(sumaGreen / 9);
            output[indexAux].blue = (unsigned char)(sumaBlue / 9);
        }
    }
}


void sobelOperator(Pixel *pixels, int width, int height, BMPHeader head) {
    Pixel *sobel_pixels_x = (Pixel *)malloc(width * height * sizeof(Pixel));
    Pixel *sobel_pixels_y = (Pixel *)malloc(width * height * sizeof(Pixel));

    convolutionSobel(pixels, width, height, sobel_x, sobel_pixels_x, head);
    convolutionSobel(pixels, width, height, sobel_y, sobel_pixels_y, head);

    // Calcular el gradiente y almacenar los resultados en una nueva matriz
    Pixel *sobel_pixels = (Pixel *)malloc(width * height * sizeof(Pixel));
    for (int i = 0; i < width * height; ++i) {
        int gradient = (int)sqrt(sobel_pixels_x[i].red * sobel_pixels_x[i].red + sobel_pixels_y[i].red * sobel_pixels_y[i].red);
        sobel_pixels[i].red = sobel_pixels[i].green = sobel_pixels[i].blue = gradient;
    }

    // Guardar la imagen resultante en el archivo de salida
    safeImageBMP(sobel_pixels, outputFilePathCS, width, height, head);

    // Liberar memoria
    free(sobel_pixels_x);
    free(sobel_pixels_y);
    free(sobel_pixels);
}



