#include <stdio.h>
#include <windows.h>
#include <sys/time.h>
#include <time.h>
#include "stdint.h"

uint32_t xor_hex(uint32_t num1, uint32_t num2);

int main() {
    char portName[] = "\\\\.\\COM3"; // Nombre del puerto COM3
    uint32_t tokenXor, tokenHex;
    char tokenHexString[50];
    unsigned char buffer[10];
    uint32_t clave = 0xB2FBCD23;
    uint8_t tokenEscrito = 0;
    struct timeval start;
    unsigned long time_milisec;

    HANDLE hSerial;

    printf("Pulse enter para inciar la sincorinisacion \n");
    if (getchar() == '\n') {

        gettimeofday(&start, NULL);
        time_milisec = start.tv_sec;
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
        buffer[0] = (unsigned char) ((time_milisec >> 24) & 0xFF);
        buffer[1] = (unsigned char) ((time_milisec >> 16) & 0xFF);
        buffer[2] = (unsigned char) ((time_milisec >> 8) & 0xFF);
        buffer[3] = (unsigned char) (time_milisec & 0xFF);
        buffer[4] = ',';
        buffer[5] = (unsigned char) ((clave >> 24) & 0xFF);
        buffer[6] = (unsigned char) ((clave >> 16) & 0xFF);
        buffer[7] = (unsigned char) ((clave >> 8) & 0xFF);
        buffer[8] = (unsigned char) (clave & 0xFF);
        buffer[9] = '\n';
        if (!WriteFile(hSerial, buffer, strlen(buffer), &bytesWrite, NULL)) {
            fprintf(stderr, "Error al escribir en el puerto. Código de error: %d\n", GetLastError());
        } else {
            printf("Tiempo %lu ms\n", time_milisec);

            printf("Datos escritos correctamente en el puerto.\n");
            // Se cierra el puerto
            CloseHandle(hSerial);
            printf("Puerto COM3 cerrado correctamente.\n");
            printf("Digite su token\n");
            scanf("%s", tokenHexString);
            tokenEscrito = 1;
        }

    }
    while (1) {
        if (tokenEscrito == 0) {
            printf("Digite su token\n");
            scanf("%s", tokenHexString);
            tokenEscrito = 1;
        } else {
            gettimeofday(&start, NULL);
            tokenHex = strtoul(tokenHexString, NULL, 16);
            tokenXor = xor_hex(tokenHex, clave);
            if (tokenXor >= (start.tv_sec - (5 * 3600) -30) && tokenXor <= (start.tv_sec - (5 * 3600))) {
                printf("Token Valido\n");
            } else {
                printf("Token Invalido\n");
            }
            tokenEscrito = 0;
        }
    }


}

uint32_t xor_hex(uint32_t num1, uint32_t num2) {
    return num1 ^ num2;
}
