#ifndef INTERFAZ_CLIENTE_H
#define INTERFAZ_CLIENTE_H

#include <ncurses.h>
#include "comunes.h"

int menu_sistema_cliente(void);

/* MEMORIA COMPARTIDA: Operaciones de lectura y escritura estructurada sobre los segmentos virtuales del cliente */
int mostrar_catalogo_productos(ProductoSHM *lista_productos, int total_productos, ItemCarrito *items_carrito, int *total_carrito);
int mostrar_carrito_compras(ItemCarrito *items_carrito, int total_items, ProductoSHM *lista_productos, int total_productos);
int formulario_perfil(char *nombre, char *apellido, char *correo, char *contrasena);

#endif