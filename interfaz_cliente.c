#include <string.h>
#include "interfaz_cliente.h"
#include "interfaz_comun.h"

int menu_sistema_cliente(void) {
    const char *opciones[] = {
        "Ver catalogo de productos", 
        "Ver carrito de compras", 
        "Ver / Modificar perfil", 
        "Cerrar sesion activa"
    };
    int num_opciones = 4;
    int resaltado = 0;
    int caracter;

    curs_set(0); 
    while (1) {
        clear();
        bkgd(COLOR_PAIR(1));
        
        int alto = 14, ancho = 38;
        if (!verificar_dimensiones(alto, ancho)) {
            caracter = getch();
            if (caracter == 27) return 4;
            continue;
        }

        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        wbkgd(ventana, COLOR_PAIR(3));
        box(ventana, 0, 0);

        dibujar_titulo_encuadrado(ventana, 1, ancho, " PANEL DE ATENCIÓN ", 4, 1);

        for (int i = 0; i < num_opciones; i++) {
            if (i == resaltado) {
                wattron(ventana, COLOR_PAIR(5) | A_BOLD);
                mvwprintw(ventana, 5 + (i * 2), 3, " > %-29s ", opciones[i]);
                wattroff(ventana, COLOR_PAIR(5) | A_BOLD);
            } else {
                wattron(ventana, COLOR_PAIR(4));
                mvwprintw(ventana, 5 + (i * 2), 3, "   %-29s ", opciones[i]);
                wattroff(ventana, COLOR_PAIR(4));
            }
        }
        
        refresh();
        wrefresh(ventana);

        caracter = wgetch(ventana);
        if (caracter == KEY_RESIZE) { delwin(ventana); continue; }
        if (caracter == KEY_UP || caracter == KEY_BTAB) resaltado = (resaltado - 1 + num_opciones) % num_opciones;
        else if (caracter == KEY_DOWN || caracter == '\t') resaltado = (resaltado + 1) % num_opciones;
        else if (caracter == '\n' || caracter == '\r') {
            delwin(ventana);
            return resaltado + 1;
        } else if (caracter == 27) {
            delwin(ventana);
            return 4; 
        }
    }
}

int mostrar_catalogo_productos(ProductoSHM *lista_productos, int total_productos, ItemCarrito *items_carrito, int *total_carrito) {
    int resaltado = 0;
    int caracter;
    curs_set(0);

    while (1) {
        clear();
        bkgd(COLOR_PAIR(1));
        
        if (!verificar_dimensiones(10, 50)) {
            caracter = getch();
            if (caracter == 27) return 0;
            continue;
        }

        int alto = LINES - 4, ancho = COLS - 8;
        int inicio_y = 2, inicio_x = 4;

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        wbkgd(ventana, COLOR_PAIR(3));
        box(ventana, 0, 0);

        dibujar_titulo_encuadrado(ventana, 1, ancho, " CATÁLOGO DE ARTÍCULOS ", 4, 1);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 5, 2, "%-4s %-26s %-10s %-5s", "ID", "Platillo / Bebida", "Precio", "Stock");
        mvwhline(ventana, 6, 2, '-', ancho - 4);

        /* MEMORIA COMPARTIDA: Renderizado secuencial desde las estructuras de la RAM privada */
        for (int i = 0; i < total_productos; i++) {
            if (i == resaltado) wattron(ventana, COLOR_PAIR(5) | A_BOLD);
            else wattron(ventana, COLOR_PAIR(4));

            mvwprintw(ventana, 7 + i, 2, "%-4d %-26.26s $%-9.2f %-5d", 
                     lista_productos[i].id_producto, 
                     lista_productos[i].nombre, 
                     lista_productos[i].precio, 
                     lista_productos[i].stock);

            if (i == resaltado) wattroff(ventana, COLOR_PAIR(5) | A_BOLD);
            else wattroff(ventana, COLOR_PAIR(4));
        }

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, alto - 2, 2, "Carro: %d | [ESC] Salir | [ENTER] Elegir", *total_carrito);
        wattroff(ventana, COLOR_PAIR(4));

        refresh();
        wrefresh(ventana);

        caracter = wgetch(ventana);
        if (caracter == KEY_RESIZE) { delwin(ventana); continue; }
        if (caracter == 27) { delwin(ventana); return 0; }
        else if (caracter == KEY_UP) resaltado = (resaltado - 1 + total_productos) % total_productos;
        else if (caracter == KEY_DOWN) resaltado = (resaltado + 1) % total_productos;
        else if (caracter == '\n' || caracter == '\r') {
            if (lista_productos[resaltado].stock <= 0) {
                mostrar_ventana_emergente("Error: Este articulo no cuenta con existencias");
                continue;
            }
            
            int cantidad = 1;
            curs_set(1);
            
            int f_alto = 8, f_ancho = 50;
            if (!verificar_dimensiones(f_alto, f_ancho)) {
                getch();
                continue;
            }
            int f_y = (LINES - f_alto) / 2, f_x = (COLS - f_ancho) / 2;
            
            attron(COLOR_PAIR(2));
            for (int i = 1; i <= f_alto; i++) mvprintw(f_y + i, f_x + f_ancho, "██");
            for (int j = 2; j <= f_ancho + 1; j++) mvprintw(f_y + f_alto, f_x + j, "▀");
            attroff(COLOR_PAIR(2));

            WINDOW *v_cant = newwin(f_alto, f_ancho, f_y, f_x);
            keypad(v_cant, TRUE);
            wbkgd(v_cant, COLOR_PAIR(3));
            box(v_cant, 0, 0);
            dibujar_titulo_encuadrado(v_cant, 1, f_ancho, " CANTIDAD DE PEDIDO ", 4, 1);
            wattron(v_cant, COLOR_PAIR(4) | A_BOLD);
            mvwprintw(v_cant, 5, 4, "Cantidad para %s:", lista_productos[resaltado].nombre);
            wattron(v_cant, COLOR_PAIR(5));
            mvwprintw(v_cant, 6, 4, "[    ]");
            wattroff(v_cant, COLOR_PAIR(5) | COLOR_PAIR(4) | A_BOLD);
            
            refresh();
            wrefresh(v_cant);
            wmove(v_cant, 6, 6);
            
            echo();
            mvwscanw(v_cant, 6, 6, "%d", &cantidad);
            noecho();
            curs_set(0);
            delwin(v_cant);

            if (cantidad <= 0 || cantidad > lista_productos[resaltado].stock) {
                mostrar_ventana_emergente("Error: Cantidad superior a existencias de barra");
            } else {
                if (*total_carrito >= MAX_ITEMS_CARRITO) {
                    mostrar_ventana_emergente("Error: El listado local se encuentra lleno");
                    continue;
                }
                
                int existe = -1;
                for(int k=0; k < *total_carrito; k++) {
                    if (items_carrito[k].id_producto == lista_productos[resaltado].id_producto) {
                        existe = k;
                        break;
                    }
                }

                if (existe != -1) {
                    if (items_carrito[existe].cantidad + cantidad > lista_productos[resaltado].stock) {
                         mostrar_ventana_emergente("Error: La suma excede el stock de cocina");
                    } else {
                         items_carrito[existe].cantidad += cantidad;
                         items_carrito[existe].subtotal = items_carrito[existe].cantidad * lista_productos[resaltado].precio;
                         mostrar_ventana_emergente("Comanda local actualizada");
                    }
                } else {
                    int idx = *total_carrito;
                    items_carrito[idx].id_producto = lista_productos[resaltado].id_producto;
                    items_carrito[idx].cantidad = cantidad;
                    items_carrito[idx].subtotal = cantidad * lista_productos[resaltado].precio;
                    (*total_carrito)++;
                    mostrar_ventana_emergente("Articulo añadido a la comanda local");
                }
            }
        }
    }
}

int mostrar_carrito_compras(ItemCarrito *items_carrito, int total_items, ProductoSHM *lista_productos, int total_productos) {
    curs_set(0);
    if (total_items == 0) {
        mostrar_ventana_emergente("La comanda local se encuentra vacia");
        return 0;
    }

    while (1) {
        clear();
        bkgd(COLOR_PAIR(1));
        
        if (!verificar_dimensiones(10, 50)) {
            int c = getch();
            if (c == 27) return 0;
            continue;
        }

        int alto = LINES - 6, ancho = COLS - 10;
        int inicio_y = 3, inicio_x = 5;

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        wbkgd(ventana, COLOR_PAIR(3));
        box(ventana, 0, 0);

        dibujar_titulo_encuadrado(ventana, 1, ancho, " COMANDA DIGITAL ", 4, 1);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 5, 2, "%-26s %-10s %-10s", "Articulo Seleccionado", "Cantidad", "Subtotal");
        mvwhline(ventana, 6, 2, '-', ancho - 4);

        float total_pagar = 0.0;
        for (int i = 0; i < total_items; i++) {
            char nombre_prod[MAX_TEXTO] = "Platillo Desconocido";
            /* MEMORIA COMPARTIDA: Lectura relacional de nombres cruzando ids de estructuras */
            for (int j = 0; j < total_productos; j++) {
                if (lista_productos[j].id_producto == items_carrito[i].id_producto) {
                    strcpy(nombre_prod, lista_productos[j].nombre);
                    break;
                }
            }
            wattron(ventana, COLOR_PAIR(4));
            mvwprintw(ventana, 7 + i, 2, "%-26.26s %-10d $%-9.2f", nombre_prod, items_carrito[i].cantidad, items_carrito[i].subtotal);
            total_pagar += items_carrito[i].subtotal;
        }

        wattron(ventana, COLOR_PAIR(5) | A_BOLD);
        mvwprintw(ventana, alto - 4, 2, " TOTAL CUENTA: $%-10.2f ", total_pagar);
        
        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, alto - 2, 2, "[C] Comprar | [ESC] Salir");
        wattroff(ventana, COLOR_PAIR(4) | COLOR_PAIR(5) | A_BOLD);

        refresh();
        wrefresh(ventana);
        int c = wgetch(ventana);
        if (c == KEY_RESIZE) { delwin(ventana); continue; }
        if (c == 27) { delwin(ventana); return 0; } 
        if (c == 'c' || c == 'C') { delwin(ventana); return 1; }
    }
}

int formulario_perfil(char *nombre, char *apellido, char *correo, char *contrasena) {
    int foco = 0;
    int c;
    int l_nom = strlen(nombre), l_ape = strlen(apellido), l_cor = strlen(correo), l_pass = strlen(contrasena);

    while(1) {
        clear();
        bkgd(COLOR_PAIR(1));
        
        int alto = 20, ancho = 46;
        if (!verificar_dimensiones(alto, ancho)) {
            c = getch();
            if (c == 27) return 0;
            continue;
        }

        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        wbkgd(ventana, COLOR_PAIR(3));
        box(ventana, 0, 0);

        dibujar_titulo_encuadrado(ventana, 1, ancho, " PERFIL ", 4, 1);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 5, 4, "Nombre:");
        if (foco == 0) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 6, 4, "[ %-36s ]", nombre);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 8, 4, "Apellido:");
        if (foco == 1) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 9, 4, "[ %-36s ]", apellido);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 11, 4, "Correo:");
        if (foco == 2) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 12, 4, "[ %-36s ]", correo);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 14, 4, "Nueva Contraseña:");
        if (foco == 3) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 15, 4, "[ ");
        for (int i = 0; i < 36; i++) { if (i < l_pass) waddch(ventana, '*'); else waddch(ventana, ' '); }
        wprintw(ventana, " ]");

        if (foco == 4) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 18, 4, " [ Guardar ] ");

        if (foco == 5) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 18, 26, " [ Cancelar ] ");
        wattroff(ventana, COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(6));

        if (foco == 0) { curs_set(1); wmove(ventana, 6, 6 + l_nom); }
        else if (foco == 1) { curs_set(1); wmove(ventana, 9, 6 + l_ape); }
        else if (foco == 2) { curs_set(1); wmove(ventana, 12, 6 + l_cor); }
        else if (foco == 3) { curs_set(1); wmove(ventana, 15, 6 + l_pass); }
        else { curs_set(0); }

        refresh();
        wrefresh(ventana);
        c = wgetch(ventana);
        if (c == KEY_RESIZE) { delwin(ventana); continue; }

        if (c == KEY_UP || c == KEY_BTAB) foco = (foco - 1 + 6) % 6;
        else if (c == KEY_DOWN || c == '\t') foco = (foco + 1) % 6;
        else if (c == KEY_LEFT && foco == 5) foco = 4;
        else if (c == KEY_RIGHT && foco == 4) foco = 5;
        else if (c == '\n' || c == '\r') {
            if (foco == 4) { delwin(ventana); return 1; }
            if (foco == 5) { delwin(ventana); return 0; }
            foco++;
        } else if (c == 27) { delwin(ventana); return 0; }
        else if (c == KEY_BACKSPACE || c == 127 || c == '\b') {
            if(foco==0 && l_nom>0) nombre[--l_nom]='\0';
            else if(foco==1 && l_ape>0) apellido[--l_ape]='\0';
            else if(foco==2 && l_cor>0) correo[--l_cor]='\0';
            else if(foco==3 && l_pass>0) contrasena[--l_pass]='\0';
        } else if (c >= 32 && c <= 126 && c != ':') {
            if(foco==0 && l_nom < MAX_USUARIO - 1) { nombre[l_nom++]=c; nombre[l_nom]='\0'; }
            else if(foco==1 && l_ape < MAX_USUARIO - 1) { apellido[l_ape++]=c; apellido[l_ape]='\0'; }
            else if(foco==2 && l_cor < MAX_TEXTO - 1) { correo[l_cor++]=c; correo[l_cor]='\0'; }
            else if(foco==3 && l_pass < MAX_USUARIO - 1) { contrasena[l_pass++]=c; contrasena[l_pass]='\0'; }
        }
    }
}