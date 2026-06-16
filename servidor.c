#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>
#include "comunes.h"
#include "cifrado.h"

#define S_BG    "\033[48;5;236m"
#define S_CYAN  "\033[38;5;51m"
#define S_GREEN "\033[38;5;46m"
#define S_WARN  "\033[38;5;226m"
#define S_ERR   "\033[38;5;196m"
#define S_MAG   "\033[38;5;201m"
#define S_DIM   "\033[38;5;244m"
#define S_BOLD  "\033[1m"
#define S_RST   "\033[0m"

void mostrar_banner_servidor() {
    printf(S_BG"\n");
    printf("    ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗ \n");
    printf("    ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗\n");
    printf("    ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝\n");
    printf("    ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗\n");
    printf("    ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║\n");
    printf("    ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝\n" S_RST);
    printf(S_BG "            SERVIDOR DEL SISTEMA ESTA ACTIVO  " S_RST "\n\n");
}

void servidor_log(const char* level, const char* color, int tid, int pid, const char* format, ...) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    printf(S_DIM "[%02d:%02d:%02d]" S_RST " %s%-7s" S_RST " | " S_MAG "TID:%-5d" S_RST " | " S_CYAN "PID:%-5d" S_RST " | ",
           local->tm_hour, local->tm_min, local->tm_sec, color, level, tid, pid);
           
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void inicializar_archivos_sistema(void) {
    FILE *archivo_usuarios = fopen("usuarios.txt", "r");
    if (archivo_usuarios == NULL) {
        archivo_usuarios = fopen("usuarios.txt", "w");
        if (archivo_usuarios != NULL) {
            char contrasena_admin[MAX_CIFRADO];
            cifrar_contrasena("Admin123!", contrasena_admin);
            fprintf(archivo_usuarios, "Administrador:General:admin@cafeteria.com:admin:%s\n", contrasena_admin);
            fclose(archivo_usuarios);
        }
    } else {
        fclose(archivo_usuarios);
    }

    FILE *archivo_articulos = fopen("articulos.txt", "r");
    if (archivo_articulos == NULL) {
        archivo_articulos = fopen("articulos.txt", "w");
        if (archivo_articulos != NULL) {
            fprintf(archivo_articulos, "1:Cafes de Extraccion Fria:45.50:100\n");
            fprintf(archivo_articulos, "2:Capuccino de Grano Organico:55.00:80\n");
            fprintf(archivo_articulos, "3:Espresso de Tueste Oscuro:35.00:120\n");
            fprintf(archivo_articulos, "4:Panini de Tres Quesos:75.00:40\n");
            fprintf(archivo_articulos, "5:Rebanada de Pastel Red Velvet:60.00:25\n");
            fclose(archivo_articulos);
        }
    } else {
        fclose(archivo_articulos);
    }

    FILE *archivo_ventas = fopen("ventas.txt", "r");
    if (archivo_ventas == NULL) {
        archivo_ventas = fopen("ventas.txt", "w");
        if (archivo_ventas != NULL) {
            fclose(archivo_ventas);
        }
    } else {
        fclose(archivo_ventas);
    }
}

typedef struct {
    key_t llave_shm;
    key_t llave_sem;
    pid_t pid_cliente;
} ArgumentosHilo;

void *atender_cliente(void *argumentos) {
    ArgumentosHilo *datos = (ArgumentosHilo *)argumentos;
    key_t shm_privada = datos->llave_shm;
    key_t sem_privado = datos->llave_sem;
    pid_t pid = datos->pid_cliente;
    free(datos);

    int shmid = shmget(shm_privada, sizeof(MemoriaPrivada), 0777);
    if (shmid == -1) {
        pthread_exit(NULL);
    }
    MemoriaPrivada *memoria = (MemoriaPrivada *)shmat(shmid, 0, 0);

    int semid = semget(sem_privado, 2, 0777);

    int tid = (int)syscall(SYS_gettid);

    servidor_log("INFO", S_CYAN, tid, pid, "❖ Canal privado bidireccional establecido con el cliente.\n");

    while (1) {
        if (down_canal(semid, 0) == -1) {
            break;
        }

        memoria->latido_servidor++;
        memoria->estado_respuesta = 0;
        memset(memoria->mensaje_alerta, 0, sizeof(memoria->mensaje_alerta));

        servidor_log("INFO", S_CYAN, tid, pid, "❖ Peticion recibida: %s\n", 
               memoria->tipo == PETICION_LOGIN ? "AUTENTICACIÓN (LOGIN)" :
               memoria->tipo == PETICION_REGISTRO ? "REGISTRO DE USUARIO" :
               memoria->tipo == PETICION_VER_PRODUCTOS ? "CONSULTA DE ARTÍCULOS (CATÁLOGO)" :
               memoria->tipo == PETICION_VER_CARRITO ? "CONSULTA DE ARTÍCULOS (CARRITO)" :
               memoria->tipo == PETICION_COMPRAR_CARRITO ? "COMPRA DE CARRITO (COMANDA)" :
               memoria->tipo == PETICION_VER_PERFIL ? "CONSULTA DE PERFIL" :
               memoria->tipo == PETICION_MODIFICAR_PERFIL ? "MODIFICACIÓN DE PERFIL" :
               memoria->tipo == PETICION_ADMIN_LISTAR_USUARIOS ? "ADMIN: LISTAR USUARIOS" :
               memoria->tipo == PETICION_ADMIN_AGREGAR_PRODUCTO ? "ADMIN: REGISTRAR PRODUCTO" :
               memoria->tipo == PETICION_ADMIN_REPORTE_VENTAS ? "ADMIN: REPORTE DE VENTAS" : 
               memoria->tipo == PETICION_LOGOUT ? "CIERRE DE SESIÓN (LOGOUT)" : "PETICION NO ESPECIFICADA");

        if (memoria->tipo == PETICION_LOGIN) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ Validando credenciales para el usuario: '%s'\n", memoria->payload.auth.usuario);
            FILE *archivo = fopen("usuarios.txt", "r");
            int verificado = 0;
            if (archivo != NULL) {
                char linea[350];
                char nom[MAX_TEXTO], ape[MAX_TEXTO], cor[MAX_TEXTO], usr[MAX_USUARIO], pass[MAX_CIFRADO];
                while (fgets(linea, sizeof(linea), archivo) != NULL) {
                    if (sscanf(linea, "%29[^:]:%29[^:]:%49[^:]:%29[^:]:%60[^\n]", nom, ape, cor, usr, pass) == 5) {
                        if (strcmp(usr, memoria->payload.auth.usuario) == 0 && strcmp(pass, memoria->payload.auth.contrasena_cifrada) == 0) {
                            strcpy(memoria->payload.auth.nombre, nom);
                            strcpy(memoria->payload.auth.apellido, ape);
                            strcpy(memoria->payload.auth.correo, cor);
                            verificado = 1;
                            break;
                        }
                    }
                }
                fclose(archivo);
            }
            if (verificado) {
                memoria->estado_respuesta = 1;
                servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ LOGIN EXITOSO para usuario '%s'. Datos de perfil transmitidos.\n", memoria->payload.auth.usuario);
            } else {
                memoria->estado_respuesta = -1;
                strcpy(memoria->mensaje_alerta, "Credenciales incorrectas o usuario inexistente");
                servidor_log("ALERTA", S_ERR, tid, pid, "✘ LOGIN FALLIDO para usuario '%s'. Motivo: %s\n", memoria->payload.auth.usuario, memoria->mensaje_alerta);
            }
        } 
        else if (memoria->tipo == PETICION_VER_PERFIL) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ Buscando datos de perfil para el usuario: '%s'\n", memoria->payload.auth.usuario);
            FILE *archivo = fopen("usuarios.txt", "r");
            int encontrado = 0;
            if (archivo != NULL) {
                char linea[350];
                char nom[MAX_TEXTO], ape[MAX_TEXTO], cor[MAX_TEXTO], usr[MAX_USUARIO], pass[MAX_CIFRADO];
                while (fgets(linea, sizeof(linea), archivo) != NULL) {
                    if (sscanf(linea, "%29[^:]:%29[^:]:%49[^:]:%29[^:]:%60[^\n]", nom, ape, cor, usr, pass) == 5) {
                        if (strcmp(usr, memoria->payload.auth.usuario) == 0) {
                            strcpy(memoria->payload.auth.nombre, nom);
                            strcpy(memoria->payload.auth.apellido, ape);
                            strcpy(memoria->payload.auth.correo, cor);
                            strcpy(memoria->payload.auth.contrasena_cifrada, pass); 
                            encontrado = 1;
                            break;
                        }
                    }
                }
                fclose(archivo);
            }
            if (encontrado) {
                memoria->estado_respuesta = 1;
                servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ PERFIL TRANSMITIDO: Datos de '%s' cargados de forma segura.\n", memoria->payload.auth.usuario);
            } else {
                memoria->estado_respuesta = -1;
                strcpy(memoria->mensaje_alerta, "Usuario no registrado");
                servidor_log("ALERTA", S_ERR, tid, pid, "✘ CONSULTA FALLIDA: El usuario '%s' no existe.\n", memoria->payload.auth.usuario);
            }
        }
        else if (memoria->tipo == PETICION_REGISTRO) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ Intentando registrar usuario: '%s' (%s %s)\n", memoria->payload.auth.usuario, memoria->payload.auth.nombre, memoria->payload.auth.apellido);
            FILE *archivo_lectura = fopen("usuarios.txt", "r");
            int duplicado = 0;
            if (archivo_lectura != NULL) {
                char linea[350];
                char nom[MAX_TEXTO], ape[MAX_TEXTO], cor[MAX_TEXTO], usr[MAX_USUARIO], pass[MAX_CIFRADO];
                while (fgets(linea, sizeof(linea), archivo_lectura) != NULL) {
                    if (sscanf(linea, "%29[^:]:%29[^:]:%49[^:]:%29[^:]:%60[^\n]", nom, ape, cor, usr, pass) == 5) {
                        if (strcmp(usr, memoria->payload.auth.usuario) == 0) {
                            duplicado = 1;
                            break;
                        }
                    }
                }
                fclose(archivo_lectura);
            }

            if (duplicado) {
                memoria->estado_respuesta = -1;
                strcpy(memoria->mensaje_alerta, "El nombre de usuario ya se encuentra registrado");
                servidor_log("ALERTA", S_ERR, tid, pid, "✘ REGISTRO RECHAZADO: El nombre de usuario '%s' ya existe.\n", memoria->payload.auth.usuario);
            } else {
                FILE *archivo_escritura = fopen("usuarios.txt", "a");
                if (archivo_escritura != NULL) {
                    fprintf(archivo_escritura, "%s:%s:%s:%s:%s\n", 
                            memoria->payload.auth.nombre, 
                            memoria->payload.auth.apellido, 
                            memoria->payload.auth.correo, 
                            memoria->payload.auth.usuario, 
                            memoria->payload.auth.contrasena_cifrada);
                    fclose(archivo_escritura);
                    memoria->estado_respuesta = 1;
                    servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ REGISTRO EXITOSO: Guardado usuario '%s' en usuarios.txt.\n", memoria->payload.auth.usuario);
                } else {
                    memoria->estado_respuesta = -1;
                    strcpy(memoria->mensaje_alerta, "Error interno de archivos del servidor");
                    servidor_log("ERROR", S_ERR, tid, pid, "✘ REGISTRO FALLIDO: No se pudo escribir en el almacenamiento.\n");
                }
            }
        }
        else if (memoria->tipo == PETICION_VER_PRODUCTOS || memoria->tipo == PETICION_VER_CARRITO) {
            if (memoria->tipo == PETICION_VER_PRODUCTOS) {
                servidor_log("INFO", S_CYAN, tid, pid, "❖ Extrayendo lista de articulos para catalogo publico...\n");
            } else {
                servidor_log("INFO", S_CYAN, tid, pid, "❖ Extrayendo catalogo para validar de forma local la comanda...\n");
            }

            FILE *archivo = fopen("articulos.txt", "r");
            int contador = 0;
            if (archivo != NULL) {
                char linea[200];
                while (fgets(linea, sizeof(linea), archivo) != NULL && contador < MAX_PRODUCTOS_CATALOGO) {
                    sscanf(linea, "%d:%49[^:]:%f:%d", 
                           &memoria->payload.catalogo.lista[contador].id_producto,
                           memoria->payload.catalogo.lista[contador].nombre,
                           &memoria->payload.catalogo.lista[contador].precio,
                           &memoria->payload.catalogo.lista[contador].stock);
                    contador++;
                }
                fclose(archivo);
                memoria->payload.catalogo.total_articulos = contador;
                memoria->estado_respuesta = 1;
                
                if (memoria->tipo == PETICION_VER_PRODUCTOS) {
                    servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ CATALOGO ENVIADO: %d productos depositados en memoria compartida.\n", contador);
                } else {
                    servidor_log("INFO", S_CYAN, tid, pid, "❖ REFERENCIA ENVIADA (%d productos en catalogo) para renderizado local.\n", contador);
                }
            } else {
                memoria->estado_respuesta = -1;
                strcpy(memoria->mensaje_alerta, "No se pudo leer el archivo de articulos");
                servidor_log("ERROR", S_ERR, tid, pid, "✘ ERROR: Fallo al abrir articulos.txt.\n");
            }
        }
        else if (memoria->tipo == PETICION_COMPRAR_CARRITO) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ Procesando comanda entrante. %d articulos en lista.\n", memoria->payload.carrito.total_items);
            FILE *archivo_art = fopen("articulos.txt", "r");
            ProductoSHM inventario_local[MAX_PRODUCTOS_CATALOGO];
            int total_inv = 0;
            
            if (archivo_art != NULL) {
                char linea[200];
                while (fgets(linea, sizeof(linea), archivo_art) != NULL) {
                    sscanf(linea, "%d:%49[^:]:%f:%d", 
                           &inventario_local[total_inv].id_producto,
                           inventario_local[total_inv].nombre,
                           &inventario_local[total_inv].precio,
                           &inventario_local[total_inv].stock);
                    total_inv++;
                }
                fclose(archivo_art);

                int stock_valido = 1;
                for (int i = 0; i < memoria->payload.carrito.total_items; i++) {
                    for (int j = 0; j < total_inv; j++) {
                        if (inventario_local[j].id_producto == memoria->payload.carrito.items[i].id_producto) {
                            
                            servidor_log("INFO", S_CYAN, tid, pid, "  Item: ID %-3d | Solicitado: %d | Stock actual: %d\n", 
                                   inventario_local[j].id_producto, memoria->payload.carrito.items[i].cantidad, inventario_local[j].stock);
                            
                            if (inventario_local[j].stock < memoria->payload.carrito.items[i].cantidad) {
                                stock_valido = 0;
                            }
                            break;
                        }
                    }
                }

                if (!stock_valido) {
                    memoria->estado_respuesta = -1;
                    strcpy(memoria->mensaje_alerta, "Error: Inconsistencia de stock durante la compra");
                    servidor_log("ALERTA", S_ERR, tid, pid, "✘ COMPRA RECHAZADA: No hay suficiente stock para completar la comanda.\n");
                } else {
                    FILE *archivo_ventas = fopen("ventas.txt", "a");
                    long tiempo_actual = (long)time(NULL);
                    float total_comanda = 0.0;
                    
                    for (int i = 0; i < memoria->payload.carrito.total_items; i++) {
                        for (int j = 0; j < total_inv; j++) {
                            if (inventario_local[j].id_producto == memoria->payload.carrito.items[i].id_producto) {
                                inventario_local[j].stock -= memoria->payload.carrito.items[i].cantidad;
                                total_comanda += memoria->payload.carrito.items[i].subtotal;
                                if (archivo_ventas != NULL) {
                                    fprintf(archivo_ventas, "%ld:%d:%d:%.2f\n", 
                                            tiempo_actual, 
                                            memoria->payload.carrito.items[i].id_producto, 
                                            memoria->payload.carrito.items[i].cantidad, 
                                            memoria->payload.carrito.items[i].subtotal);
                                }
                                break;
                            }
                        }
                    }
                    if (archivo_ventas != NULL) fclose(archivo_ventas);

                    FILE *archivo_art_w = fopen("articulos.txt", "w");
                    if (archivo_art_w != NULL) {
                        for (int k = 0; k < total_inv; k++) {
                            fprintf(archivo_art_w, "%d:%s:%.2f:%d\n", 
                                    inventario_local[k].id_producto, 
                                    inventario_local[k].nombre, 
                                    inventario_local[k].precio, 
                                    inventario_local[k].stock);
                        }
                        fclose(archivo_art_w);
                    }
                    memoria->estado_respuesta = 1;
                    servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ COMPRA CONSOLIDADA: Cuenta de $%.2f registrada en ventas.txt e inventario actualizado.\n", total_comanda);
                }
            } else {
                memoria->estado_respuesta = -1;
                strcpy(memoria->mensaje_alerta, "Error de acceso al archivo de inventario");
                servidor_log("ERROR", S_ERR, tid, pid, "✘ ERROR: No se pudo abrir articulos.txt.\n");
            }
        }
        else if (memoria->tipo == PETICION_MODIFICAR_PERFIL) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ Procesando modificacion de perfil para usuario: '%s'\n", memoria->payload.auth.usuario);
            FILE *archivo_r = fopen("usuarios.txt", "r");
            char bloques_usuarios[50][350];
            int total_u = 0, modificado = 0;
            
            if (archivo_r != NULL) {
                char linea[350];
                char nom[MAX_TEXTO], ape[MAX_TEXTO], cor[MAX_TEXTO], usr[MAX_USUARIO], pass[MAX_CIFRADO];
                while (fgets(linea, sizeof(linea), archivo_r) != NULL) {
                    sscanf(linea, "%29[^:]:%29[^:]:%49[^:]:%29[^:]:%60[^\n]", nom, ape, cor, usr, pass);
                    if (strcmp(usr, memoria->payload.auth.usuario) == 0) {
                        sprintf(bloques_usuarios[total_u], "%s:%s:%s:%s:%s\n", 
                                memoria->payload.auth.nombre, 
                                memoria->payload.auth.apellido, 
                                memoria->payload.auth.correo, 
                                usr, 
                                memoria->payload.auth.contrasena_cifrada);
                        modificado = 1;
                    } else {
                        strcpy(bloques_usuarios[total_u], linea);
                    }
                    total_u++;
                }
                fclose(archivo_r);

                if (modificado) {
                    FILE *archivo_w = fopen("usuarios.txt", "w");
                    if (archivo_w != NULL) {
                        for (int i = 0; i < total_u; i++) {
                            fputs(bloques_usuarios[i], archivo_w);
                        }
                        fclose(archivo_w);
                    }
                    memoria->estado_respuesta = 1;
                    servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ PERFIL ACTUALIZADO: Cambios guardados para '%s' (Correo: %s).\n", memoria->payload.auth.usuario, memoria->payload.auth.correo);
                } else {
                    memoria->estado_respuesta = -1;
                    strcpy(memoria->mensaje_alerta, "No se localizo el registro de usuario");
                    servidor_log("ERROR", S_ERR, tid, pid, "✘ ERROR: No se encontro el usuario '%s' en la base de datos.\n", memoria->payload.auth.usuario);
                }
            }
        }
        else if (memoria->tipo == PETICION_ADMIN_LISTAR_USUARIOS) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ ADMIN: Listando usuarios registrados en el sistema...\n");
            FILE *archivo = fopen("usuarios.txt", "r");
            memset(memoria->payload.reportes.desglose_reporte, 0, sizeof(memoria->payload.reportes.desglose_reporte));
            if (archivo != NULL) {
                char linea[350];
                char nom[MAX_TEXTO], ape[MAX_TEXTO], cor[MAX_TEXTO], usr[MAX_USUARIO], pass[MAX_CIFRADO];
                char acumulador[2048] = "";
                while (fgets(linea, sizeof(linea), archivo) != NULL) {
                    if (sscanf(linea, "%29[^:]:%29[^:]:%49[^:]:%29[^:]:%60[^\n]", nom, ape, cor, usr, pass) == 5) {
                        char formateado[256];
                        snprintf(formateado, sizeof(formateado), "Usuario: %-12s | %s %s (%s)\n", usr, nom, ape, cor);
                        strcat(acumulador, formateado);
                    }
                }
                fclose(archivo);
                strcpy(memoria->payload.reportes.desglose_reporte, acumulador);
                memoria->estado_respuesta = 1;
                servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ ADMIN: Listado de usuarios transmitido exitosamente por el canal privado.\n");
            }
        }
        else if (memoria->tipo == PETICION_ADMIN_AGREGAR_PRODUCTO) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ ADMIN: Agregando nuevo producto: '%s' | Precio: $%.2f | Stock: %d\n", 
                   memoria->payload.catalogo.lista[0].nombre, 
                   memoria->payload.catalogo.lista[0].precio, 
                   memoria->payload.catalogo.lista[0].stock);
            
            FILE *archivo_lectura = fopen("articulos.txt", "r");
            int ultimo_id = 0;
            if (archivo_lectura != NULL) {
                char linea[200];
                int id_temp;
                while (fgets(linea, sizeof(linea), archivo_lectura) != NULL) {
                    if (sscanf(linea, "%d:", &id_temp) == 1) {
                        if (id_temp > ultimo_id) ultimo_id = id_temp;
                    }
                }
                fclose(archivo_lectura);
            }
            
            FILE *archivo_append = fopen("articulos.txt", "a");
            if (archivo_append != NULL) {
                fprintf(archivo_append, "%d:%s:%.2f:%d\n", 
                        ultimo_id + 1, 
                        memoria->payload.catalogo.lista[0].nombre, 
                        memoria->payload.catalogo.lista[0].precio, 
                        memoria->payload.catalogo.lista[0].stock);
                fclose(archivo_append);
                memoria->estado_respuesta = 1;
                servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ ADMIN: Producto '%s' agregado exitosamente con ID %d.\n", 
                       memoria->payload.catalogo.lista[0].nombre, ultimo_id + 1);
            }
        }
        else if (memoria->tipo == PETICION_ADMIN_REPORTE_VENTAS) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ ADMIN: Calculando reportes contables de ingresos...\n");
            FILE *archivo_ventas = fopen("ventas.txt", "r");
            float suma_diaria = 0, suma_semanal = 0, suma_mensual = 0;
            char buffer_desglose[2048] = "";
            long tiempo_actual = (long)time(NULL);

            if (archivo_ventas != NULL) {
                char linea[150];
                long timestamp;
                int id_p, cant;
                float sub;
                while (fgets(linea, sizeof(linea), archivo_ventas) != NULL) {
                    if (sscanf(linea, "%ld:%d:%d:%f", &timestamp, &id_p, &cant, &sub) == 4) {
                        long diferencia = tiempo_actual - timestamp;
                        if (diferencia <= 86400) suma_diaria += sub;
                        if (diferencia <= 604800) suma_semanal += sub;
                        if (diferencia <= 2592000) suma_mensual += sub;

                        char fila[100];
                        sprintf(fila, "Venta de Articulo ID %d | Cantidad: %d | Subtotal: $%.2f\n", id_p, cant, sub);
                        strcat(buffer_desglose, fila);
                    }
                }
                fclose(archivo_ventas);
                memoria->payload.reportes.total_diario = suma_diaria;
                memoria->payload.reportes.total_semanal = suma_semanal;
                memoria->payload.reportes.total_mensual = suma_mensual;
                strcpy(memoria->payload.reportes.desglose_reporte, buffer_desglose);
                memoria->estado_respuesta = 1;
                servidor_log("ÉXITO", S_GREEN, tid, pid, "✔ ADMIN: Reporte enviado (Diario: $%.2f | Semanal: $%.2f | Mensual: $%.2f).\n", 
                       suma_diaria, suma_semanal, suma_mensual);
            }
        }
        else if (memoria->tipo == PETICION_LOGOUT) {
            servidor_log("INFO", S_CYAN, tid, pid, "❖ SESIÓN CERRADA para el usuario: '%s'. Canal liberado.\n", memoria->payload.auth.usuario);
            memoria->estado_respuesta = 1;
        }

        up_canal(semid, 1);
    }

    servidor_log("INFO", S_CYAN, tid, pid, "❖ El cliente ha cerrado la conexion. Desvinculando segmento.\n");
    shmdt(memoria);
    pthread_exit(NULL);
}

int main(void) {
    inicializar_archivos_sistema();

    key_t llave_shm_publica = ftok(RUTA_BASE, ID_CANAL_PUBLICO);
    int shmid_publico = shmget(llave_shm_publica, sizeof(CanalPublico), IPC_CREAT | 0777);
    CanalPublico *canal_publico = (CanalPublico *)shmat(shmid_publico, 0, 0);
    
    canal_publico->estatus = 0;
    canal_publico->pid_solicitante = 0;

    key_t llave_sem_clientes = ftok(RUTA_BASE, 'C');
    key_t llave_sem_mutex = ftok(RUTA_BASE, ID_MUTEX_PUBLICO);
    
    int sem_clientes = crear_semaforo(llave_sem_clientes, 1, 0);
    int sem_mutex = crear_semaforo(llave_sem_mutex, 1, 1);

    pthread_attr_t atributos;
    pthread_attr_init(&atributos);
    pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

    mostrar_banner_servidor();
    printf(S_GREEN " [✔] Sistema Inicializado Correctamente\n" S_RST);
    printf(S_WARN " [⧗] Escuchando Terminales IPC por el Canal Central...\n" S_RST);
    printf("\n\n");

    while (1) {
        down_canal(sem_clientes, 0);
        down_canal(sem_mutex, 0);

        if (canal_publico->estatus == 1) {
            pid_t pid_solicitante = canal_publico->pid_solicitante;
            
            time_t now;
            time(&now);
            struct tm *local = localtime(&now);
            printf(S_DIM "[%02d:%02d:%02d]" S_RST " %s%-7s" S_RST " | " S_MAG "TID: MAIN" S_RST " | " S_CYAN "PID: ---- " S_RST " | ❖ Nueva peticion entrante desde PID: %d\n", 
                    local->tm_hour, local->tm_min, local->tm_sec, S_CYAN, "PETICION", pid_solicitante);

            key_t nueva_shm_privada = ftok(RUTA_BASE, 'a' + (pid_solicitante % 25));
            key_t nuevo_sem_privado = ftok(RUTA_BASE, 'a' + ((pid_solicitante + 1) % 25));

            int shmid_privado = shmget(nueva_shm_privada, sizeof(MemoriaPrivada), IPC_CREAT | 0777);
            MemoriaPrivada *mem_privada = (MemoriaPrivada *)shmat(shmid_privado, 0, 0);
            mem_privada->pid_cliente = pid_solicitante;
            mem_privada->latido_servidor = 1;
            mem_privada->estado_respuesta = 0;
            shmdt(mem_privada);

            crear_semaforo(nuevo_sem_privado, 2, 0);

            canal_publico->llave_shm_privada = nueva_shm_privada;
            canal_publico->llave_sem_privado = nuevo_sem_privado;
            canal_publico->estatus = 2; 

            ArgumentosHilo *args = (ArgumentosHilo *)malloc(sizeof(ArgumentosHilo));
            args->llave_shm = nueva_shm_privada;
            args->llave_sem = nuevo_sem_privado;
            args->pid_cliente = pid_solicitante;

            pthread_t id_hilo_trabajador;
            pthread_create(&id_hilo_trabajador, &atributos, atender_cliente, (void *)args);
        }

        up_canal(sem_mutex, 0);
    }

    shmdt(canal_publico);
    shmctl(shmid_publico, IPC_RMID, 0);
    return 0;
}