#include <stdio.h>
#include <windows.h>
#include <sys/time.h>
#include <time.h>

int xor_hex(int num1, int num2);

int main() {

    struct timeval current_time;
    struct tm *timeinfo;
    char f[50];
    char fResult[50];

    int x = xor_hex(0x0000162E, 0xF10ABCD0);
    //printf("xor %x ", x);
    HANDLE hSerial;
    char portName[] = "\\\\.\\COM3"; // Nombre del puerto COM3
    unsigned char buffer[10];
    unsigned int cont = 0;
    unsigned long clave = 0xB2FBCD23;
    unsigned long inputToken = 0 ;
    struct timeval start;
    struct tm *time;

    printf("Pulse enter para inciar la sincorinisacion \n");
    if (getchar() == '\n') {

        gettimeofday(&start, NULL);
        unsigned long time_milisec = start.tv_sec;
        // Intentar abrir el puer.
        hSerial = CreateFile(portName,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

        if (hSerial == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error al abrir el puerto. Código de error: %d\n", GetLastError());
            return 1;
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams)) {
            fprintf(stderr, "Error al obtener los parámetros del puerto. Código de error: %d\n", GetLastError());
            CloseHandle(hSerial);
            return 1;
        }

        dcbSerialParams.BaudRate = CBR_9600; // Velocidad de transmisión (9600 baudios)
        dcbSerialParams.ByteSize = 8; // Tamaño del byte
        dcbSerialParams.StopBits = ONESTOPBIT; // 1 bit de parada
        dcbSerialParams.Parity = NOPARITY; // Sin paridad

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            fprintf(stderr, "Error al configurar los parámetros del puerto. Código de error: %d\n", GetLastError());
            CloseHandle(hSerial);
            return 1;
        }

        printf("Puerto COM3 abierto correctamente.\n");

        // Se agregar el código para comunicarte con el puerto

        DWORD bytesWrite;
        buffer[0] = (unsigned char)((time_milisec >> 24) & 0xFF);
        buffer[1] = (unsigned char)((time_milisec >> 16) & 0xFF);
        buffer[2] = (unsigned char)((time_milisec >> 8) & 0xFF);
        buffer[3] = (unsigned char)(time_milisec & 0xFF);
        buffer[4] = ',';
        buffer[5] = (unsigned char)((clave >> 24) & 0xFF);
        buffer[6] = (unsigned char)((clave >> 16) & 0xFF);
        buffer[7] = (unsigned char)((clave >> 8) & 0xFF);
        buffer[8] = (unsigned char)(clave & 0xFF);
        buffer[9] = '\n';
        if (!WriteFile(hSerial, buffer, strlen(buffer), &bytesWrite, NULL)) {
            fprintf(stderr, "Error al escribir en el puerto. Código de error: %d\n", GetLastError());
        } else {
            printf("Tiempo %lu ms\n", time_milisec);

            printf("Datos escritos correctamente en el puerto.\n");
            // Se cierra el puerto
            CloseHandle(hSerial);
            printf("Puerto COM3 cerrado correctamente.\n");
        }

    }
    printf("ingrese su token \n");
    scanf("%lu", inputToken);
    cont = 1;
    while (1) {

        if(inputToken != 0){
            cont = 1;
        }
        if(cont == 1){
            gettimeofday(&current_time, NULL);
            time_t x = current_time.tv_sec /1000;
            sprintf(f, "%x", x);
            int result = xor_hex((unsigned long)f,clave);
            sprintf(fResult, "%x", result);
            if (fResult == inputToken){
                printf("Token valido\n");
            }else{
                printf("Token invalido\n");
            }
            cont = 0;
            printf("ingrese su token \n");
            scanf("%lu", inputToken);
        }


        /*time_t raw_time = current_time.tv_sec; // Convierte tv_sec a time_t 5fe43593
         *

        // Convierte el time_t a una estructura tm
        timeinfo = localtime(&raw_time);

        // Extrae los campos de hora, minutos y segundos
        int hour = timeinfo->tm_hour;
        int minute = timeinfo->tm_min;
        int second = timeinfo->tm_sec;

        printf("Hora: %02d:%02d:%02d\n", hour, minute, second);*/
    }

}

int xor_hex(int num1, int num2) {
    return num1 ^ num2;
}
