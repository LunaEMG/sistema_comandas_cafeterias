#include <string.h>
#include "validacion.h"

int validar_correo(const char *correo) {
    const char *arroba = strchr(correo, '@');
    if (!arroba || arroba == correo) {
        return 0;
    }
    
    const char *punto = strchr(arroba, '.');
    if (!punto || punto == arroba + 1 || *(punto + 1) == '\0') {
        return 0;
    }
    
    if (contiene_caracter_invalido(correo)) {
        return 0;
    }

    return 1;
}

int validar_contrasena_segura(const char *contrasena) {
    if (strlen(contrasena) < 8) {
        return 0;
    }
    
    if (contiene_caracter_invalido(contrasena)) {
        return 0;
    }

    int tiene_letra = 0;
    int tiene_numero = 0;
    int tiene_especial = 0;
    
    for (int indice = 0; contrasena[indice] != '\0'; indice++) {
        if ((contrasena[indice] >= 'a' && contrasena[indice] <= 'z') || (contrasena[indice] >= 'A' && contrasena[indice] <= 'Z')) {
            tiene_letra = 1;
        } else if (contrasena[indice] >= '0' && contrasena[indice] <= '9') {
            tiene_numero = 1;
        } else {
            tiene_especial = 1;
        }
    }
    return (tiene_letra && tiene_numero && tiene_especial);
}

int contiene_caracter_invalido(const char *texto) {
    if (strchr(texto, ':') != NULL) {
        return 1;
    }
    if (strchr(texto, '\n') != NULL) {
        return 1;
    }
    return 0;
}