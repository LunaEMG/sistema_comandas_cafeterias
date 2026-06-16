#define _DEFAULT_SOURCE
#include <string.h>
#include <locale.h>
#include "comunes.h"      
#include "interfaz_comun.h"
#include <stdlib.h>
#include "validacion.h"

void iniciar_interfaz(void) {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(0); 
    curs_set(1);
    
    if (has_colors() && can_change_color()) {
        start_color();
        
        init_color(10, 120, 115, 110); 
        init_color(11, 50, 45, 40);    
        init_color(12, 443, 349, 290); 
        init_color(13, 341, 223, 160); 
        init_color(14, 850, 811, 682); 
        
        init_pair(1, COLOR_WHITE, 10); 
        init_pair(2, 11, 10);          
        init_pair(3, 12, 13);          
        init_pair(4, 14, 13);          
        init_pair(5, COLOR_BLACK, 14); 
        init_pair(6, 14, COLOR_BLACK); 

        init_color(15, 341, 223, 160); 
        init_color(16, 850, 811, 682); 
        init_color(17, 850, 811, 682); 
        init_color(18, 200, 200, 200); 

        init_pair(7, COLOR_WHITE, 15);  
        init_pair(8, COLOR_BLACK, 16);  
        init_pair(9, 16, 15);           
        init_pair(10, 17, 15);          
        init_pair(11, COLOR_RED, 15);   

        init_pair(12, COLOR_RED, 13);   
    }
}

void cerrar_interfaz(void) {
    endwin();
}

void dibujar_titulo_encuadrado(WINDOW *win, int y, int ancho_ventana, const char *titulo, int par_color, int es_negrita) {
    int len = mbstowcs(NULL, titulo, 0);
    if (len == -1) len = strlen(titulo);
    int box_width = len + 4;
    int start_x = (ancho_ventana - box_width) / 2;
    
    if (es_negrita) {
        wattron(win, COLOR_PAIR(par_color) | A_BOLD);
    } else {
        wattron(win, COLOR_PAIR(par_color));
    }
    
    mvwprintw(win, y, start_x, "┌");
    for (int i = 0; i < len + 2; i++) {
        waddstr(win, "─");
    }
    waddstr(win, "┐");
    
    mvwprintw(win, y + 1, start_x, "│ %s │", titulo);
    
    mvwprintw(win, y + 2, start_x, "└");
    for (int i = 0; i < len + 2; i++) {
        waddstr(win, "─");
    }
    waddstr(win, "┘");
    
    if (es_negrita) {
        wattroff(win, COLOR_PAIR(par_color) | A_BOLD);
    } else {
        wattroff(win, COLOR_PAIR(par_color));
    }
}

int verificar_dimensiones(int min_alto, int min_ancho) {
    if (LINES < min_alto || COLS < min_ancho) {
        clear();
        bkgd(COLOR_PAIR(1));
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(LINES / 2, (COLS > 35) ? (COLS - 35) / 2 : 0, "Terminal muy pequeña. Redimensione.");
        attroff(COLOR_PAIR(1) | A_BOLD);
        refresh();
        return 0;
    }
    return 1;
}

int menu_principal(void) {
    const char *opciones[] = {"Inicio de sesion", "Registro de usuario", "Salir del sistema"};
    int num_opciones = 3;
    int resaltado = 0;
    int caracter;

    curs_set(0); 
    while (1) {
        int alto = 24, ancho = 62;
        if (!verificar_dimensiones(alto, ancho)) {
            caracter = getch();
            if (caracter == 27) return 3;
            continue;
        }

        clear();
        bkgd(COLOR_PAIR(1));
        
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

        wattron(ventana, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(ventana, 1,  (ancho - 54) / 2, "                  █     █     █                      ");
        mvwprintw(ventana, 2,  (ancho - 54) / 2, "                  █     █     █                      ");
        mvwprintw(ventana, 3,  (ancho - 54) / 2, "                   █     █     █                     ");
        mvwprintw(ventana, 4,  (ancho - 54) / 2, "                                                     ");
        mvwprintw(ventana, 5,  (ancho - 54) / 2, "               ████████████████████                  ");
        mvwprintw(ventana, 6,  (ancho - 54) / 2, "               ██                █████               ");
        mvwprintw(ventana, 7,  (ancho - 54) / 2, "               ██                ██  █               ");
        mvwprintw(ventana, 8,  (ancho - 54) / 2, "               ██                ██  █               ");
        mvwprintw(ventana, 9,  (ancho - 54) / 2, "               ██                █████               ");
        mvwprintw(ventana, 10, (ancho - 54) / 2, "                 ██            ██                    ");
        mvwprintw(ventana, 11, (ancho - 54) / 2, "                   ████████████                      ");
        mvwprintw(ventana, 12, (ancho - 54) / 2, "            __  __             ____              _   ");
        mvwprintw(ventana, 13, (ancho - 54) / 2, "  ___ ___  / _|/ _| ___  ___  / ___| _ __   ___ | |_ ");
        mvwprintw(ventana, 14, (ancho - 54) / 2, " / __/ _ \\| |_| |_ / _ \\/ _ \\ \\___ \\| '_ \\ / _ \\| __|");
        mvwprintw(ventana, 15, (ancho - 54) / 2, "| (_| (_) |  _|  _|  __/  __/  ___) | |_) | (_) | |_ ");
        mvwprintw(ventana, 16, (ancho - 54) / 2, " \\___\\___/|_| |_|  \\___|\\___| |____/| .__/ \\___/ \\__|");
        mvwprintw(ventana, 17, (ancho - 54) / 2, "                                    |_|              ");
        wattroff(ventana, COLOR_PAIR(4) | A_BOLD);

        for (int i = 0; i < num_opciones; i++) {
            if (i == resaltado) {
                wattron(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK);
                mvwprintw(ventana, 19 + i, (ancho - 32) / 2, " > %-28s ", opciones[i]);
                wattroff(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK);
            } else {
                wattron(ventana, COLOR_PAIR(4));
                mvwprintw(ventana, 19 + i, (ancho - 32) / 2, "   %-28s ", opciones[i]);
                wattroff(ventana, COLOR_PAIR(4));
            }
        }
        
        refresh();
        wrefresh(ventana);
        
        caracter = wgetch(ventana);
        
        if (caracter == KEY_RESIZE) {
            delwin(ventana);
            continue; 
        }
        
        if (caracter == KEY_UP || caracter == KEY_BTAB) resaltado = (resaltado - 1 + num_opciones) % num_opciones;
        else if (caracter == KEY_DOWN || caracter == '\t') resaltado = (resaltado + 1) % num_opciones;
        else if (caracter == '\n' || caracter == '\r') {
            delwin(ventana);
            return resaltado + 1;
        } else if (caracter == 27) {
            delwin(ventana);
            return 3; 
        }
    }
}

int formulario_credenciales(const char *titulo, char *usuario, int max_usr, char *contrasena, int max_pass) {
    int foco = 0; 
    int caracter;
    int len_usr = strlen(usuario);
    int len_pass = strlen(contrasena);
    int es_admin = (strstr(titulo, "ADMINISTRADOR") != NULL);

    while (1) {
        int alto = es_admin ? 24 : 18;
        int ancho = es_admin ? 60 : 46;
        if (!verificar_dimensiones(alto, ancho)) {
            caracter = getch();
            if (caracter == 27) return 0;
            continue;
        }

        clear();
        bkgd(COLOR_PAIR(1));
        
        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));

        WINDOW *ventana = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana, TRUE);
        if (es_admin) wbkgd(ventana, COLOR_PAIR(9));
        else wbkgd(ventana, COLOR_PAIR(3));
        box(ventana, 0, 0);

        if (es_admin) {
            wattron(ventana, COLOR_PAIR(10) | A_BOLD);
            int start_y = 2;
            int start_x = (ancho - 53) / 2;
            mvwprintw(ventana, start_y, start_x,     "       _   ___  __  __ ___ _  _         ");
            mvwprintw(ventana, start_y + 1, start_x, "      /_\\ |   \\|  \\/  |_ _| \\| |        ");
            mvwprintw(ventana, start_y + 2, start_x, "     / _ \\| |) | |\\/| || || .` |        ");
            mvwprintw(ventana, start_y + 3, start_x, "    /_/ \\_\\___/|_|  |_|___|_|\\_|        ");
            mvwprintw(ventana, start_y + 4, start_x, "            __  __             ____              _   ");
            mvwprintw(ventana, start_y + 5, start_x, "  ___ ___  / _|/ _| ___  ___  / ___| _ __   ___ | |_ ");
            mvwprintw(ventana, start_y + 6, start_x, " / __/ _ \\| |_| |_ / _ \\/ _ \\ \\___ \\| '_ \\ / _ \\| __|");
            mvwprintw(ventana, start_y + 7, start_x,"| (_| (_) |  _|  _|  __/  __/  ___) | |_) | (_) | |_ ");
            mvwprintw(ventana, start_y + 8, start_x," \\___\\___/|_| |_|  \\___|\\___| |____/| .__/ \\___/ \\__|");
            mvwprintw(ventana, start_y + 9, start_x,"                                    |_|              ");
            wattroff(ventana, COLOR_PAIR(10) | A_BOLD);
        }

        if (!es_admin) {
            dibujar_titulo_encuadrado(ventana, 1, ancho, titulo, 4, 1);
        }

        int shift_y = es_admin ? 8 : 0;
        int shift_x = es_admin ? (ancho - 46) / 2 : 0;

        if (es_admin) wattron(ventana, COLOR_PAIR(10)); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 5 + shift_y, 6 + shift_x, "Nombre de Usuario:");
        
        if (foco == 0) {
            if (es_admin) wattron(ventana, COLOR_PAIR(8) | A_BOLD);
            else wattron(ventana, COLOR_PAIR(5) | A_BOLD);
        } else {
            wattron(ventana, COLOR_PAIR(6));
        }
        mvwprintw(ventana, 6 + shift_y, 6 + shift_x, "[ %-30s ]", usuario);
        if (foco == 0) {
            if (es_admin) wattroff(ventana, COLOR_PAIR(8) | A_BOLD);
            else wattroff(ventana, COLOR_PAIR(5) | A_BOLD);
        } else {
            wattroff(ventana, COLOR_PAIR(6));
        }

        if (es_admin) wattron(ventana, COLOR_PAIR(10)); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 9 + shift_y, 6 + shift_x, "Contraseña de Acceso:");
        
        if (foco == 1) {
            if (es_admin) wattron(ventana, COLOR_PAIR(8) | A_BOLD);
            else wattron(ventana, COLOR_PAIR(5) | A_BOLD);
        } else {
            wattron(ventana, COLOR_PAIR(6));
        }
        mvwprintw(ventana, 10 + shift_y, 6 + shift_x, "[ ");
        for (int i = 0; i < 30; i++) {
            if (i < len_pass) waddch(ventana, '*');
            else waddch(ventana, ' ');
        }
        wprintw(ventana, " ]");
        if (foco == 1) {
            if (es_admin) wattroff(ventana, COLOR_PAIR(8) | A_BOLD);
            else wattroff(ventana, COLOR_PAIR(5) | A_BOLD);
        } else {
            wattroff(ventana, COLOR_PAIR(6));
        }

        if (foco == 2) {
            if (es_admin) wattron(ventana, COLOR_PAIR(8) | A_BOLD | A_BLINK);
            else wattron(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK);
        } else {
            if (es_admin) wattron(ventana, COLOR_PAIR(10)); else wattron(ventana, COLOR_PAIR(4));
        }
        mvwprintw(ventana, 14 + shift_y, 6 + shift_x, " [ Entrar ] ");
        wattroff(ventana, COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(8) | COLOR_PAIR(10) | A_BLINK);

        if (foco == 3) {
            if (es_admin) wattron(ventana, COLOR_PAIR(8) | A_BOLD | A_BLINK);
            else wattron(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK);
        } else {
            if (es_admin) wattron(ventana, COLOR_PAIR(10)); else wattron(ventana, COLOR_PAIR(4));
        }
        mvwprintw(ventana, 14 + shift_y, 26 + shift_x, " [ Salir ] ");
        wattroff(ventana, COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(8) | COLOR_PAIR(10) | A_BLINK);

        if (foco == 0) { curs_set(1); wmove(ventana, 6 + shift_y, 8 + shift_x + len_usr); }
        else if (foco == 1) { curs_set(1); wmove(ventana, 10 + shift_y, 8 + shift_x + len_pass); }
        else { curs_set(0); }

        refresh();
        wrefresh(ventana);
        caracter = wgetch(ventana);

        if (caracter == KEY_RESIZE) {
            delwin(ventana);
            continue;
        }

        if (caracter == KEY_UP || caracter == KEY_BTAB) foco = (foco - 1 + 4) % 4;
        else if (caracter == KEY_DOWN || caracter == '\t') foco = (foco + 1) % 4;
        else if (caracter == KEY_LEFT && foco == 3) foco = 2;
        else if (caracter == KEY_RIGHT && foco == 2) foco = 3;
        else if (caracter == '\n' || caracter == '\r') {
            if (foco == 2) { delwin(ventana); curs_set(1); return 1; }
            if (foco == 3) { delwin(ventana); curs_set(1); return 0; }
            foco++; 
        } else if (caracter == 27) { delwin(ventana); return 0; } 
        else if (caracter == KEY_BACKSPACE || caracter == 127 || caracter == '\b') {
            if (foco == 0 && len_usr > 0) usuario[--len_usr] = '\0';
            else if (foco == 1 && len_pass > 0) contrasena[--len_pass] = '\0';
        } else if (caracter >= 32 && caracter <= 126 && caracter != ':') { 
            if (foco == 0 && len_usr < max_usr - 1) {
                usuario[len_usr++] = caracter; usuario[len_usr] = '\0';
            } else if (foco == 1 && len_pass < max_pass - 1) {
                contrasena[len_pass++] = caracter; contrasena[len_pass] = '\0';
            }
        }
    }
}

int formulario_registro(char *nombre, char *apellido, char *correo, char *usuario, char *contrasena) {
    int foco = 0; 
    int c;
    int l_nom = strlen(nombre), l_ape = strlen(apellido), l_cor = strlen(correo), l_usr = strlen(usuario), l_pass = strlen(contrasena);

    while(1) {
        int alto = 24, ancho = 48;
        if (!verificar_dimensiones(alto, ancho)) {
            c = getch();
            if (c == 27) return 0;
            continue;
        }

        clear();
        bkgd(COLOR_PAIR(1));
        
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

        dibujar_titulo_encuadrado(ventana, 1, ancho, " REGISTRO ", 4, 1);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 5, 4, "Nombre:");
        if (foco == 0) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 6, 4, "[ %-38s ]", nombre);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 8, 4, "Apellido:");
        if (foco == 1) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 9, 4, "[ %-38s ]", apellido);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 11, 4, "Correo Electronico:");
        if (foco == 2) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 12, 4, "[ %-38s ]", correo);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 14, 4, "Nombre de Usuario:");
        if (foco == 3) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 15, 4, "[ %-38s ]", usuario);

        wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 17, 4, "Contraseña de Acceso:");
        if (foco == 4) wattron(ventana, COLOR_PAIR(5) | A_BOLD); else wattron(ventana, COLOR_PAIR(6));
        mvwprintw(ventana, 18, 4, "[ ");
        for (int i = 0; i < 38; i++) { if (i < l_pass) waddch(ventana, '*'); else waddch(ventana, ' '); }
        wprintw(ventana, " ]");

        if (foco == 5) wattron(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 21, 4, " [ Registrar ] ");
        wattroff(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK);

        if (foco == 6) wattron(ventana, COLOR_PAIR(5) | A_BOLD | A_BLINK); else wattron(ventana, COLOR_PAIR(4));
        mvwprintw(ventana, 21, 28, " [ Cancelar ] ");
        wattroff(ventana, COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(6) | A_BLINK);

        if (foco == 0) { curs_set(1); wmove(ventana, 6, 6 + l_nom); }
        else if (foco == 1) { curs_set(1); wmove(ventana, 9, 6 + l_ape); }
        else if (foco == 2) { curs_set(1); wmove(ventana, 12, 6 + l_cor); }
        else if (foco == 3) { curs_set(1); wmove(ventana, 15, 6 + l_usr); }
        else if (foco == 4) { curs_set(1); wmove(ventana, 18, 6 + l_pass); }
        else { curs_set(0); }

        refresh();
        wrefresh(ventana);
        c = wgetch(ventana);

        if (c == KEY_RESIZE) {
            delwin(ventana);
            continue;
        }

        int nuevo_foco = foco;

        if (c == KEY_UP || c == KEY_BTAB) {
            if (foco == 0) nuevo_foco = 6; 
            else if (foco == 5 || foco == 6) nuevo_foco = 4; 
            else nuevo_foco = foco - 1;
        } else if (c == KEY_DOWN || c == '\t') {
            if (foco == 4) nuevo_foco = 5; 
            else if (foco == 5 || foco == 6) nuevo_foco = 0; 
            else nuevo_foco = foco + 1;
        } else if (c == KEY_LEFT) {
            if (foco == 6) nuevo_foco = 5;
        } else if (c == KEY_RIGHT) {
            if (foco == 5) nuevo_foco = 6;
        } else if (c == '\n' || c == '\r') {
            if (foco == 5) { delwin(ventana); return 1; }
            if (foco == 6) { delwin(ventana); return 0; }
            nuevo_foco = foco + 1; 
        } else if (c == 27) { 
            delwin(ventana); return 0; 
        } else if (c == KEY_BACKSPACE || c == 127 || c == '\b') {
            if (foco == 0 && l_nom > 0) nombre[--l_nom] = '\0';
            else if (foco == 1 && l_ape > 0) apellido[--l_ape] = '\0';
            else if (foco == 2 && l_cor > 0) correo[--l_cor] = '\0';
            else if (foco == 3 && l_usr > 0) usuario[--l_usr] = '\0';
            else if (foco == 4 && l_pass > 0) contrasena[--l_pass] = '\0';
        } else if (c >= 32 && c <= 126 && c != ':') { 
            if (foco == 0 && l_nom < MAX_USUARIO - 1) { nombre[l_nom++] = c; nombre[l_nom] = '\0'; }
            else if (foco == 1 && l_ape < MAX_USUARIO - 1) { apellido[l_ape++] = c; apellido[l_ape] = '\0'; }
            else if (foco == 2 && l_cor < MAX_TEXTO - 1) { correo[l_cor++] = c; correo[l_cor] = '\0'; }
            else if (foco == 3 && l_usr < MAX_USUARIO - 1) { usuario[l_usr++] = c; usuario[l_usr] = '\0'; }
            else if (foco == 4 && l_pass < MAX_USUARIO - 1) { contrasena[l_pass++] = c; contrasena[l_pass] = '\0'; }
        }

        if (foco == 2 && nuevo_foco != 2 && l_cor > 0) {
            if (!validar_correo(correo)) {
                mostrar_ventana_emergente("Error: Correo electronico invalido");
                nuevo_foco = 2; 
            }
        }

        if (foco == 4 && nuevo_foco != 4 && l_pass > 0) {
            if (!validar_contrasena_segura(contrasena)) {
                mostrar_ventana_emergente(
                    "Error: Contraseña insegura.\n"
                    "Requisitos mínimos de seguridad:\n"
                    "- Mínimo 8 caracteres de longitud\n"
                    "- Al menos una letra (a-z, A-Z)\n"
                    "- Al menos un número (0-9)\n"
                    "- Al menos un carácter especial"
                );
                nuevo_foco = 4; 
            }
        }

        foco = nuevo_foco;
    }
}

void mostrar_ventana_emergente(const char *mensaje) {
    int lineas = 1;
    int max_ancho = 0;
    int len_temp = 0;

    for (int i = 0; mensaje[i] != '\0'; i++) {
        if (mensaje[i] == '\n') {
            lineas++;
            if (len_temp > max_ancho) max_ancho = len_temp;
            len_temp = 0;
        } else {
            len_temp++;
        }
    }
    if (len_temp > max_ancho) max_ancho = len_temp;

    int alto = 10 + lineas;
    int ancho = max_ancho + 12;
    if (ancho < 42) ancho = 42;

    while (1) {
        if (!verificar_dimensiones(alto, ancho)) {
            int c = getch();
            if (c == 27 || c == '\n' || c == '\r') return;
            continue;
        }

        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;

        clear();
        bkgd(COLOR_PAIR(1));

        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));
        refresh();

        WINDOW *ventana_modal = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(ventana_modal, TRUE);
        wbkgd(ventana_modal, COLOR_PAIR(3));
        box(ventana_modal, 0, 0);
        
        dibujar_titulo_encuadrado(ventana_modal, 1, ancho, " ATENCIÓN ", 4, 1);
        
        char copia_msg[512];
        strncpy(copia_msg, mensaje, sizeof(copia_msg) - 1); 
        copia_msg[sizeof(copia_msg) - 1] = '\0';
        
        wattron(ventana_modal, COLOR_PAIR(4) | A_BOLD);
        char *linea = strtok(copia_msg, "\n");
        int curr_y = 5; 
        while (linea != NULL) {
            mvwprintw(ventana_modal, curr_y++, (ancho - strlen(linea)) / 2, "%s", linea);
            linea = strtok(NULL, "\n");
        }
        wattroff(ventana_modal, COLOR_PAIR(4) | A_BOLD);
        
        wattron(ventana_modal, COLOR_PAIR(5) | A_BOLD | A_BLINK);
        mvwprintw(ventana_modal, alto - 2, (ancho - 13) / 2, " [ Entendido ] ");
        wattroff(ventana_modal, COLOR_PAIR(5) | A_BOLD | A_BLINK | COLOR_PAIR(4));
        
        wrefresh(ventana_modal);

        curs_set(0);
        int caracter = wgetch(ventana_modal);
        
        if (caracter == KEY_RESIZE) {
            delwin(ventana_modal);
            continue; 
        }
        
        if (caracter == '\n' || caracter == '\r' || caracter == 27) {
            delwin(ventana_modal);
            break;
        }
        delwin(ventana_modal);
    }
}

void mostrar_requisitos_contrasena(void) {
    int alto = 14, ancho = 44; 
    
    while (1) {
        if (!verificar_dimensiones(alto, ancho)) {
            int c = getch();
            if (c == 27 || c == '\n' || c == '\r') return;
            continue;
        }

        int inicio_y = (LINES - alto) / 2;
        int inicio_x = (COLS - ancho) / 2;
        
        clear();
        bkgd(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        for (int i = 1; i <= alto; i++) mvprintw(inicio_y + i, inicio_x + ancho, "██");
        for (int j = 2; j <= ancho + 1; j++) mvprintw(inicio_y + alto, inicio_x + j, "▀");
        attroff(COLOR_PAIR(2));
        refresh();

        WINDOW *win = newwin(alto, ancho, inicio_y, inicio_x);
        keypad(win, TRUE);
        wbkgd(win, COLOR_PAIR(3));
        box(win, 0, 0);
        
        dibujar_titulo_encuadrado(win, 1, ancho, " SEGURIDAD DE CLAVE ", 4, 1);
        
        wattron(win, COLOR_PAIR(4));
        mvwprintw(win, 5, 4, "- Minimo 8 caracteres de longitud");
        mvwprintw(win, 6, 4, "- Al menos una letra (a-z, A-Z)");
        mvwprintw(win, 7, 4, "- Al menos un numero (0-9)");
        mvwprintw(win, 8, 4, "- Al menos un caracter especial");
        wattroff(win, COLOR_PAIR(4));

        wattron(win, COLOR_PAIR(5) | A_BOLD | A_BLINK);
        mvwprintw(win, 11, (ancho - 11) / 2, " [ Cerrar ] ");
        wattroff(win, COLOR_PAIR(5) | A_BOLD | COLOR_PAIR(4) | A_BLINK);
        
        wrefresh(win);

        curs_set(0);
        int c = wgetch(win);
        
        if (c == KEY_RESIZE) {
            delwin(win);
            continue;
        }
        
        if (c == '\n' || c == '\r' || c == 27) {
            delwin(win);
            break;
        }
        delwin(win);
    }
}