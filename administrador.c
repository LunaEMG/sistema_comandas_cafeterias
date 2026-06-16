#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comunes.h"
#include "interfaz.h"
#include "cifrado.h"

int main(void) {
    /* SEMAFOROS Y MEMORIA COMPARTIDA: Verificacion de la existencia de las llaves publicas globales */
    key_t llave_shm_publica = ftok(RUTA_BASE, ID_CANAL_PUBLICO);
    key_t llave_sem_clientes = ftok(RUTA_BASE, 'C');
    key_t llave_sem_mutex = ftok(RUTA_BASE, ID_MUTEX_PUBLICO);

    int sem_clientes = semget(llave_sem_clientes, 1, 0777);
    int sem_mutex = semget(llave_sem_mutex, 1, 0777);
    int shmid_publico = shmget(llave_shm_publica, sizeof(CanalPublico), 0777);

    if (sem_clientes == -1 || sem_mutex == -1 || shmid_publico == -1) {
        iniciar_interfaz();
        mostrar_ventana_emergente("Error: El servidor central no se encuentra activo");
        cerrar_interfaz();
        return 1;
    }

    /* MEMORIA COMPARTIDA: Enlace provisional al segmento publico para transaccion Handshake */
    CanalPublico *canal_publico = (CanalPublico *)shmat(shmid_publico, 0, 0);

    /* SEMAFOROS: Bloqueo de exclusion mutua seguro con validacion de error inmediato */
    if (down_canal(sem_mutex, 0) == -1) {
        iniciar_interfaz();
        mostrar_ventana_emergente("Error: El servidor central no se encuentra activo");
        cerrar_interfaz();
        shmdt(canal_publico);
        return 1;
    }
    
    canal_publico->pid_solicitante = getpid();
    canal_publico->estatus = 1;
    up_canal(sem_clientes, 0);

    int intentos_timeout = 0;
    int conexion_exitosa = 1;

    while (canal_publico->estatus != 2) {
        up_canal(sem_mutex, 0);
        usleep(15000); 
        intentos_timeout++;
        
        if (intentos_timeout > 100) { 
            conexion_exitosa = 0;
            break;
        }
        
        if (down_canal(sem_mutex, 0) == -1) {
            conexion_exitosa = 0;
            break;
        }
    }

    if (!conexion_exitosa) {
        iniciar_interfaz();
        mostrar_ventana_emergente("Error: El servidor central no se encuentra activo");
        cerrar_interfaz();
        shmdt(canal_publico);
        return 1;
    }

    key_t shm_privada = canal_publico->llave_shm_privada;
    key_t sem_privado = canal_publico->llave_sem_privado;
    up_canal(sem_mutex, 0);
    
    /* MEMORIA COMPARTIDA: Desconexion de la memoria publica */
    shmdt(canal_publico);

    /* MEMORIA COMPARTIDA Y SEMAFOROS: Vinculacion definitiva a la RAM y conjunto de semaforos privados de la terminal */
    int shmid_privado = shmget(shm_privada, sizeof(MemoriaPrivada), 0777);
    int semid_privado = semget(sem_privado, 2, 0777);
    
    if (shmid_privado == -1 || semid_privado == -1) {
        iniciar_interfaz();
        mostrar_ventana_emergente("Error al enlazar los canales de control gerencial");
        cerrar_interfaz();
        return 1;
    }
    MemoriaPrivada *memoria = (MemoriaPrivada *)shmat(shmid_privado, 0, 0);

    iniciar_interfaz();

    while (1) {
        char usr[MAX_USUARIO] = "";
        char pass[MAX_CONTRASENA] = "";

        int accion_inicial = formulario_credenciales("ACCESO PANEL DE ADMINISTRADOR", usr, MAX_USUARIO, pass, MAX_CONTRASENA);
        if (accion_inicial == 0) {
            break;
        }

        if (strcmp(usr, "admin") != 0) {
            mostrar_ventana_emergente("Error: Acceso restringido unicamente a roles gerenciales");
            continue;
        }

        /* MEMORIA COMPARTIDA: Registro de parametros de autenticacion en el payload privado */
        strcpy(memoria->payload.auth.usuario, usr);
        cifrar_contrasena(pass, memoria->payload.auth.contrasena_cifrada);
        memoria->tipo = PETICION_LOGIN;

        /* SEMAFOROS: Alerta al servidor por canal 0 y bloqueo de espera asincrona en canal 1 */
        up_canal(semid_privado, 0);
        
        if (down_canal(semid_privado, 1) == -1) {
            mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
            cerrar_interfaz();
            return 1;
        }

        if (memoria->estado_respuesta == 1) {
            mostrar_ventana_emergente("Autenticacion de administrador otorgada!");
            
            ProductoSHM lista_productos_local[MAX_PRODUCTOS_CATALOGO];
            int total_productos_local = 0;
            DatosReporte reportes_local;
            
            int panel_activo = 1;
            while (panel_activo) {
                /* MEMORIA COMPARTIDA: Solicitud de sincronizacion de productos para alimentar el Dashboard */
                memoria->tipo = PETICION_VER_PRODUCTOS;
                
                /* SEMAFOROS: Notificacion por canal 0 y espera de respuesta en canal 1 */
                up_canal(semid_privado, 0);
                
                if (down_canal(semid_privado, 1) == -1) {
                    mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                    cerrar_interfaz();
                    return 1;
                }
                
                if (memoria->estado_respuesta == 1) {
                    total_productos_local = memoria->payload.catalogo.total_articulos;
                    memcpy(lista_productos_local, memoria->payload.catalogo.lista, sizeof(lista_productos_local));
                }
                
                /* MEMORIA COMPARTIDA: Solicitud de sincronizacion contable para el Dashboard */
                memoria->tipo = PETICION_ADMIN_REPORTE_VENTAS;
                
                /* SEMAFOROS: Notificacion por canal 0 y espera de respuesta en canal 1 */
                up_canal(semid_privado, 0);
                
                if (down_canal(semid_privado, 1) == -1) {
                    mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                    cerrar_interfaz();
                    return 1;
                }
                
                if (memoria->estado_respuesta == 1) {
                    memcpy(&reportes_local, &memoria->payload.reportes, sizeof(DatosReporte));
                }

                int opcion_admin = menu_sistema_administrador(lista_productos_local, total_productos_local, &reportes_local);

                if (opcion_admin == 4) {
                    strcpy(memoria->payload.auth.usuario, "admin");
                    memoria->tipo = PETICION_LOGOUT;
                    up_canal(semid_privado, 0);
                    
                    if (down_canal(semid_privado, 1) == -1) {
                        mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                        cerrar_interfaz();
                        return 1;
                    }
                    panel_activo = 0;
                }
                else if (opcion_admin == 1) {
                    /* MEMORIA COMPARTIDA: Solicitud de listado estructurado de personal registrado */
                    memoria->tipo = PETICION_ADMIN_LISTAR_USUARIOS;
                    
                    /* SEMAFOROS: Notificacion por canal 0 y espera de respuesta en canal 1 */
                    up_canal(semid_privado, 0);
                    
                    if (down_canal(semid_privado, 1) == -1) {
                        mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                        cerrar_interfaz();
                        return 1;
                    }

                    if (memoria->estado_respuesta == 1) {
                        mostrar_administracion_usuarios(memoria->payload.reportes.desglose_reporte);
                    } else {
                        mostrar_ventana_emergente(memoria->mensaje_alerta);
                    }
                }
                else if (opcion_admin == 2) {
                    char n_prod[MAX_TEXTO] = "";
                    float n_prec = 0.0;
                    int n_stk = 0;

                    int confirmar = formulario_producto(n_prod, &n_prec, &n_stk, 1);
                    if (confirmar) {
                        /* MEMORIA COMPARTIDA: Registro del nuevo platillo en el payload privado */
                        strcpy(memoria->payload.catalogo.lista[0].nombre, n_prod);
                        memoria->payload.catalogo.lista[0].precio = n_prec;
                        memoria->payload.catalogo.lista[0].stock = n_stk;
                        memoria->tipo = PETICION_ADMIN_AGREGAR_PRODUCTO;

                        /* SEMAFOROS: Notificacion por canal 0 y espera de respuesta en canal 1 */
                        up_canal(semid_privado, 0);
                        
                        if (down_canal(semid_privado, 1) == -1) {
                            mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                            cerrar_interfaz();
                            return 1;
                        }

                        if (memoria->estado_respuesta == 1) {
                            mostrar_ventana_emergente("Articulo insertado con exito en el catalogo del servidor");
                        } else {
                            mostrar_ventana_emergente(memoria->mensaje_alerta);
                        }
                    }
                }
                else if (opcion_admin == 3) {
                    /* MEMORIA COMPARTIDA: Solicitud de desglose historico detallado de transacciones de caja */
                    memoria->tipo = PETICION_ADMIN_REPORTE_VENTAS;
                    
                    /* SEMAFOROS: Notificacion por canal 0 y espera de respuesta en canal 1 */
                    up_canal(semid_privado, 0);
                    
                    if (down_canal(semid_privado, 1) == -1) {
                        mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                        cerrar_interfaz();
                        return 1;
                    }

                    if (memoria->estado_respuesta == 1) {
                        mostrar_reportes_ventas(&memoria->payload.reportes);
                    } else {
                        mostrar_ventana_emergente(memoria->mensaje_alerta);
                    }
                }
            }
            break;
        } else {
            mostrar_ventana_emergente(memoria->mensaje_alerta);
            pass[0] = '\0';
        }
    }

    /* MEMORIA COMPARTIDA: Desconexion formal de la terminal al segmento virtual privado */
    shmdt(memoria);
    cerrar_interfaz();
    return 0;
}