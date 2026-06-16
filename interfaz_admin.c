#include <string.h>
#include "interfaz_admin.h"
#include "interfaz_comun.h"

/* MEMORIA COMPARTIDA: Mapeo y distribucion de paneles gráficos leyendo buffers sincronizados de la RAM privada */
int menu_sistema_administrador(ProductoSHM *lista_productos, int total_productos, DatosReporte *datos_reporte) {
    curs_set(0);
    clear();
    bkgd(COLOR_PAIR(7));

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if (!verificar_dimensiones(15, 60)) {
        int control_tecla = getch();
        if (control_tecla == 27) return 4;
        return 0;
    }

    WINDOW *top_bar = newwin(3, max_x, 0, 0);
    keypad(top_bar, TRUE);
    wbkgd(top_bar, COLOR_PAIR(8));
    mvwprintw(top_bar, 1, 2, "CONSOLA DE ADMINISTRACIÓN");
    
    int alto_paneles = max_y - 5;
    int ancho_izq = 32;
    int ancho_der = max_x - ancho_izq - 1;

    WINDOW *win_stats = newwin(alto_paneles, ancho_izq, 3, 0);
    keypad(win_stats, TRUE);
    wbkgd(win_stats, COLOR_PAIR(9));
    box(win_stats, 0, 0);
    dibujar_titulo_encuadrado(win_stats, 1, ancho_izq, " RESUMEN DEL DÍA ", 9, 1);
    
    wattron(win_stats, COLOR_PAIR(10));
    mvwprintw(win_stats, 5, 2, "Total Caja:  $%-10.2f", datos_reporte->total_diario);
    
    int num_comandas = 0;
    char copia_desglose[2048];
    strcpy(copia_desglose, datos_reporte->desglose_reporte);
    char *token_c = strtok(copia_desglose, "\n");
    while(token_c != NULL) {
        num_comandas++;
        token_c = strtok(NULL, "\n");
    }
    mvwprintw(win_stats, 6, 2, "Comandas:    %-5d", num_comandas);
    
    wattron(win_stats, COLOR_PAIR(11) | A_BOLD);
    mvwprintw(win_stats, 9, 2, "ALERTAS DE STOCK");
    wattroff(win_stats, COLOR_PAIR(11) | A_BOLD);
    
    int linea_alerta = 11;
    wattron(win_stats, COLOR_PAIR(10));
    for (int i = 0; i < total_productos && linea_alerta < alto_paneles - 2; i++) {
        char nom_alerta[16];
        if (strlen(lista_productos[i].nombre) > 12) {
            strncpy(nom_alerta, lista_productos[i].nombre, 9);
            nom_alerta[9] = '\0';
            strcat(nom_alerta, "...");
        } else {
            strcpy(nom_alerta, lista_productos[i].nombre);
        }

        if (lista_productos[i].stock <= 15) {
            mvwprintw(win_stats, linea_alerta++, 2, "- %-12s (Critico)", nom_alerta);
        } else if (lista_productos[i].stock <= 40) {
            mvwprintw(win_stats, linea_alerta++, 2, "- %-12s (Bajo)", nom_alerta);
        }
    }

    WINDOW *win_table = newwin(alto_paneles, ancho_der, 3, ancho_izq + 1);
    keypad(win_table, TRUE);
    wbkgd(win_table, COLOR_PAIR(9));
    box(win_table, 0, 0);
    dibujar_titulo_encuadrado(win_table, 1, ancho_der, " MONITOREO DE INVENTARIO ACTUAL ", 9, 1);
    
    wattron(win_table, COLOR_PAIR(8));
    mvwprintw(win_table, 5, 3, " ID   PRODUCTO                        STOCK    ESTADO      ");
    
    wattron(win_table, COLOR_PAIR(10));
    for (int i = 0; i < total_productos && i < alto_paneles - 5; i++) {
        const char *estado_stock = "Optimo";
        if (lista_productos[i].stock <= 15) estado_stock = "Critico";
        else if (lista_productos[i].stock <= 40) estado_stock = "Bajo";

        mvwprintw(win_table, 7 + i, 3, " %02d   %-30s  %-7d  %-10s ", 
                  lista_productos[i].id_producto, 
                  lista_productos[i].nombre, 
                  lista_productos[i].stock, 
                  estado_stock);
    }

    WINDOW *bot_bar = newwin(2, max_x, max_y - 2, 0);
    keypad(bot_bar, TRUE);
    wbkgd(bot_bar, COLOR_PAIR(7));
    wattron(bot_bar, COLOR_PAIR(10));
    mvwprintw(bot_bar, 0, 2, "[F2] Inventario   [F3] Reportes   [F4] Usuarios   [ESC] Cerrar Sesion");

    refresh();
    wrefresh(top_bar);
    wrefresh(win_stats);
    wrefresh(win_table);
    wrefresh(bot_bar);

    int control_tecla;
    
    while (1) {
        control_tecla = wgetch(bot_bar);
        
        if (control_tecla == KEY_RESIZE) {
            delwin(top_bar);
            delwin(win_stats);
            delwin(win_table);
            delwin(bot_bar);
            return 0;
        }
        
        if (control_tecla == KEY_F(2) || control_tecla == 'i' || control_tecla == 'I') {
            delwin(top_bar); delwin(win_stats); delwin(win_table); delwin(bot_bar);
            return 2;
        }
        if (control_tecla == KEY_F(3) || control_tecla == 'r' || control_tecla == 'R') {
            delwin(top_bar); delwin(win_stats); delwin(win_table); delwin(bot_bar);
            return 3;
        }
        if (control_tecla == KEY_F(4) || control_tecla == 'u' || control_tecla == 'U') {
            delwin(top_bar); delwin(win_stats); delwin(win_table); delwin(bot_bar);
            return 1;
        }
        if (control_tecla == 27) { // Tecla ESC
            delwin(top_bar); delwin(win_stats); delwin(win_table); delwin(bot_bar);
            return 4;
        }
        
    }
}

/* MEMORIA COMPARTIDA: Recuperacion e impresion en pantalla del listado de personal alojado en el segmento privado */
void mostrar_administracion_usuarios(const char *desglose_usuarios) {
    curs_set(0);
    clear();
    bkgd(COLOR_PAIR(7));
    
    if (!verificar_dimensiones(10, 40)) {
        int c = getch();
        if (c == KEY_RESIZE) mostrar_administracion_usuarios(desglose_usuarios);
        return;
    }

    int alto = LINES - 6, ancho = COLS - 10;
    int inicio_y = 3, inicio_x = 5;

    WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
    keypad(ventana, TRUE);
    wbkgd(ventana, COLOR_PAIR(9));
    box(ventana, 0, 0);

    dibujar_titulo_encuadrado(ventana, 1, ancho, " CONTROL DE ACCESOS ", 8, 1);
    
    int linea_y = 6;
    char copia[2048];
    strcpy(copia, desglose_usuarios);
    char *token = strtok(copia, "\n");
    
    while(token != NULL && linea_y < alto - 3) {
        wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, linea_y++, 4, "%s", token);
        token = strtok(NULL, "\n");
    }

    wattron(ventana, COLOR_PAIR(8) | A_BOLD);
    mvwprintw(ventana, alto - 2, 4, " [ Presione cualquier tecla para regresar ] ");
    wattroff(ventana, COLOR_PAIR(8) | A_BOLD);
    
    refresh();
    wrefresh(ventana);
    int c = wgetch(ventana);
    if (c == KEY_RESIZE) {
        delwin(ventana);
        mostrar_administracion_usuarios(desglose_usuarios); // Recurse to redraw
        return;
    }
    delwin(ventana);
}

int formulario_producto(char *nombre, float *precio, int *stock, int es_alta) {
    int foco = 0;
    int c;
    char str_precio[15] = "";
    char str_stock[15] = "";
    
    if(!es_alta) {
        sprintf(str_precio, "%.2f", *precio);
        sprintf(str_stock, "%d", *stock);
    }
    
    int l_nom = strlen(nombre), l_pre = strlen(str_precio), l_sto = strlen(str_stock);
    curs_set(1);

    while(1) {
        clear();
        bkgd(COLOR_PAIR(7));
        
        int alto = 18, ancho = 44;
        if (!verificar_dimensiones(alto, ancho)) {
            c = getch();
            if (c == 27) return 0;
            continue;
        }

        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        wbkgd(ventana, COLOR_PAIR(9));
        box(ventana, 0, 0);

        dibujar_titulo_encuadrado(ventana, 1, ancho, es_alta ? " ALTA DE ARTÍCULO " : " CAMBIO DE ARTÍCULO ", 8, 1);

        wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 5, 4, "Nombre del Producto:");
        if (foco == 0) wattron(ventana, COLOR_PAIR(8) | A_BOLD); else wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 6, 4, "[ %-34s ]", nombre);

        wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 8, 4, "Precio Comercial ($):");
        if (foco == 1) wattron(ventana, COLOR_PAIR(8) | A_BOLD); else wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 9, 4, "[ %-34s ]", str_precio);

        wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 11, 4, "Unidades en Inventario:");
        if (foco == 2) wattron(ventana, COLOR_PAIR(8) | A_BOLD); else wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 12, 4, "[ %-34s ]", str_stock);

        if (foco == 3) wattron(ventana, COLOR_PAIR(8) | A_BOLD); else wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 16, 4, " [ Procesar ] ");

        if (foco == 4) wattron(ventana, COLOR_PAIR(8) | A_BOLD); else wattron(ventana, COLOR_PAIR(10));
        mvwprintw(ventana, 16, 24, " [ Cancelar ] ");
        wattroff(ventana, COLOR_PAIR(8) | COLOR_PAIR(10));

        if (foco == 0) wmove(ventana, 6, 6 + l_nom);
        else if (foco == 1) wmove(ventana, 9, 6 + l_pre);
        else if (foco == 2) wmove(ventana, 12, 6 + l_sto);
        else curs_set(0);

        refresh();
        wrefresh(ventana);
        c = wgetch(ventana);
        if (c == KEY_RESIZE) { delwin(ventana); continue; }

        if (c == KEY_UP || c == KEY_BTAB) { foco = (foco - 1 + 5) % 5; curs_set(foco < 3 ? 1 : 0); }
        else if (c == KEY_DOWN || c == '\t') { foco = (foco + 1) % 5; curs_set(foco < 3 ? 1 : 0); }
        else if (c == KEY_LEFT && foco == 4) foco = 3;
        else if (c == KEY_RIGHT && foco == 3) foco = 4;
        else if (c == '\n' || c == '\r') {
            if (foco == 3) {
                if (l_nom == 0 || l_pre == 0 || l_sto == 0) {
                    mostrar_ventana_emergente("Error: Todos los campos obligatorios");
                    foco = 0; curs_set(1); continue;
                }
                sscanf(str_precio, "%f", precio);
                sscanf(str_stock, "%d", stock);
                delwin(ventana);
                return 1;
            }
            if (foco == 4) { delwin(ventana); return 0; }
            foco++; curs_set(foco < 3 ? 1 : 0);
        } else if (c == 27) { delwin(ventana); return 0; }
        else if (c == KEY_BACKSPACE || c == 127 || c == '\b') {
            if(foco==0 && l_nom>0) nombre[--l_nom]='\0';
            else if(foco==1 && l_pre>0) str_precio[--l_pre]='\0';
            else if(foco==2 && l_sto>0) str_stock[--l_sto]='\0';
        } else if (c >= 32 && c <= 126 && c != ':') {
            if(foco==0 && l_nom < MAX_TEXTO - 1) { nombre[l_nom++]=c; nombre[l_nom]='\0'; }
            else if(foco==1 && l_pre < 12 && ((c>='0' && c<='9') || c=='.')) { str_precio[l_pre++]=c; str_precio[l_pre]='\0'; }
            else if(foco==2 && l_sto < 12 && (c>='0' && c<='9')) { str_stock[l_sto++]=c; str_stock[l_sto]='\0'; }
        }
    }
}

/* MEMORIA COMPARTIDA: Volcado y despliegue del log de auditoria macroeconomico extraido del payload de la RAM */
void mostrar_reportes_ventas(const DatosReporte *datos_reporte) {
    curs_set(0);
    clear();
    bkgd(COLOR_PAIR(7));
    
    if (!verificar_dimensiones(10, 50)) {
        int c = getch();
        if (c == KEY_RESIZE) mostrar_reportes_ventas(datos_reporte);
        return;
    }

    int alto = LINES - 4, ancho = COLS - 8;
    int inicio_y = 2, inicio_x = 4;

    WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
    keypad(ventana, TRUE);
    wbkgd(ventana, COLOR_PAIR(9));
    box(ventana, 0, 0);

    dibujar_titulo_encuadrado(ventana, 1, ancho, " AUDITORÍA DE VENTAS ", 8, 1);
    
    wattron(ventana, COLOR_PAIR(10));
    mvwprintw(ventana, 5, 4, "HISTORICO GENERAL DE INGRESOS:");
    mvwprintw(ventana, 6, 4, "--------------------------------------------------");
    
    int linea_y = 8;
    char copia[2048];
    strcpy(copia, datos_reporte->desglose_reporte);
    char *token = strtok(copia, "\n");
    
    while(token != NULL && linea_y < alto - 3) {
        mvwprintw(ventana, linea_y++, 4, "%s", token);
        token = strtok(NULL, "\n");
    }

    wattron(ventana, COLOR_PAIR(8) | A_BOLD);
    mvwprintw(ventana, alto - 2, 4, " [ Presione cualquier tecla para regresar ] ");
    wattroff(ventana, COLOR_PAIR(8) | A_BOLD);
    
    refresh();
    wrefresh(ventana);
    int c = wgetch(ventana);
    if (c == KEY_RESIZE) {
        delwin(ventana);
        mostrar_reportes_ventas(datos_reporte); // Recurse to redraw
        return;
    }
    delwin(ventana);
}