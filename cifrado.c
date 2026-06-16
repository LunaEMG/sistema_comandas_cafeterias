#include <stdio.h>
#include <string.h>
#include "cifrado.h"

static const char *clave_secreta = "Sist3m4S3gur0L1nux";

void cifrar_contrasena(const char *texto_plano, char *texto_cifrado) {
    int longitud_texto = strlen(texto_plano);
    int longitud_clave = strlen(clave_secreta);
    
    for (int indice = 0; indice < longitud_texto; indice++) {
        unsigned char caracter_cifrado = texto_plano[indice] ^ clave_secreta[indice % longitud_clave];
        
        snprintf(&texto_cifrado[indice * 2], 3, "%02x", caracter_cifrado);
    }
    texto_cifrado[longitud_texto * 2] = '\0';
}