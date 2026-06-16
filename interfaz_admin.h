#ifndef INTERFAZ_ADMIN_H
#define INTERFAZ_ADMIN_H

#include <ncurses.h>
#include "comunes.h"

/* MEMORIA COMPARTIDA: Transmision de estructuras de control contable e inventario para el Dashboard */
int menu_sistema_administrador(ProductoSHM *lista_productos, int total_productos, DatosReporte *datos_reporte);

/* MEMORIA COMPARTIDA: Lectura del buffer de usuarios recuperados de la RAM privada */
void mostrar_administracion_usuarios(const char *desglose_usuarios);

int formulario_producto(char *nombre, float *precio, int *stock, int es_alta);

/* MEMORIA COMPARTIDA: Visualizacion de las estadisticas agregadas de ventas guardadas en el payload */
void mostrar_reportes_ventas(const DatosReporte *datos_reporte);

#endif