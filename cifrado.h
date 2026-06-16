#ifndef CIFRADO_H
#define CIFRADO_H

#include "comunes.h"

/* SEGURIDAD: Proteccion de credenciales antes de ser transmitidas por la memoria compartida privada */
void cifrar_contrasena(const char *texto_plano, char *texto_cifrado);

#endif