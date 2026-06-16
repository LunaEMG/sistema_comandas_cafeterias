#ifndef INTERFAZ_COMUN_H
#define INTERFAZ_COMUN_H

#include <ncurses.h>

void iniciar_interfaz(void);
void cerrar_interfaz(void);
int menu_principal(void);
int formulario_credenciales(const char *titulo, char *usuario, int max_usr, char *contrasena, int max_pass);
int formulario_registro(char *nombre, char *apellido, char *correo, char *usuario, char *contrasena);
void mostrar_ventana_emergente(const char *mensaje);
void mostrar_requisitos_contrasena(void);

void dibujar_titulo_encuadrado(WINDOW *win, int y, int ancho_ventana, const char *titulo, int par_color, int es_negrita);
int verificar_dimensiones(int min_alto, int min_ancho);

#endif