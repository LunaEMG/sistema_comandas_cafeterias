#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comunes.h"
#include "interfaz.h"
#include "cifrado.h"
#include "validacion.h"

int main(void) {
    /* SEMAFOROS Y MEMORIA COMPARTIDA: Verificacion de la existencia de las llaves publicas del servidor */
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

    /* MEMORIA COMPARTIDA: Vinculacion temporal al segmento publico para el Handshake inicial */
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
    
    /* MEMORIA COMPARTIDA: Desconexion del canal publico al obtener las llaves privadas asignadas */
    shmdt(canal_publico);

    /* MEMORIA COMPARTIDA Y SEMAFOROS: Enlace definitivo a los recursos virtuales privados de la mesa */
    int shmid_privado = shmget(shm_privada, sizeof(MemoriaPrivada), 0777);
    int semid_privado = semget(sem_privado, 2, 0777);
    
    if (shmid_privado == -1 || semid_privado == -1) {
        iniciar_interfaz();
        mostrar_ventana_emergente("Error al enlazar los canales privados de comunicacion");
        cerrar_interfaz();
        return 1;
    }
    
    MemoriaPrivada *memoria = (MemoriaPrivada *)shmat(shmid_privado, 0, 0);

    ItemCarrito carrito_local[MAX_ITEMS_CARRITO];
    int total_carrito = 0;
    char usuario_autenticado[MAX_USUARIO] = "";

    iniciar_interfaz();

    while (1) {
        int opcion_menu = menu_principal();

        if (opcion_menu == 3) {
            break;
        }

        if (opcion_menu == 1) {
            char usr[MAX_USUARIO] = "";
            char pass[MAX_CONTRASENA] = "";

            while (1) {
                int accion = formulario_credenciales("INICIO DE SESION", usr, MAX_USUARIO, pass, MAX_CONTRASENA);
                if (accion == 0) {
                    break;
                }

                if (strlen(usr) == 0 || strlen(pass) == 0) {
                    mostrar_ventana_emergente("Error: Todos los campos son obligatorios");
                    continue;
                }

                /* MEMORIA COMPARTIDA: Escritura de datos estructurados en el payload privado */
                strcpy(memoria->payload.auth.usuario, usr);
                cifrar_contrasena(pass, memoria->payload.auth.contrasena_cifrada);
                memoria->tipo = PETICION_LOGIN;

                /* SEMAFOROS: Notifica la solicitud en el INDICE 0 y espera la respuesta en el INDICE 1 */
                up_canal(semid_privado, 0);
                
                if (down_canal(semid_privado, 1) == -1) {
                    mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                    cerrar_interfaz();
                    return 1;
                }

                if (memoria->estado_respuesta == 1) {
                    strcpy(usuario_autenticado, usr);
                    mostrar_ventana_emergente("Inicio de sesion exitoso!");
                    
                    int sesion_activa = 1;
                    while (sesion_activa) {
                        int opcion_sistema = menu_sistema_cliente();

                        if (opcion_sistema == 4) {
                            strcpy(memoria->payload.auth.usuario, usuario_autenticado);
                            memoria->tipo = PETICION_LOGOUT;
                            up_canal(semid_privado, 0);
                            
                            if (down_canal(semid_privado, 1) == -1) {
                                mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                cerrar_interfaz();
                                return 1;
                            }

                            usuario_autenticado[0] = '\0';
                            total_carrito = 0;
                            sesion_activa = 0;
                        }
                        else if (opcion_sistema == 1) {
                            memoria->tipo = PETICION_VER_PRODUCTOS;
                            
                            /* SEMAFOROS: Despierta al hilo servidor por canal 0 y duerme esperando respuesta en canal 1 */
                            up_canal(semid_privado, 0);
                            
                            if (down_canal(semid_privado, 1) == -1) {
                                mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                cerrar_interfaz();
                                return 1;
                            }

                            if (memoria->estado_respuesta == 1) {
                                int comprar = mostrar_catalogo_productos(
                                    memoria->payload.catalogo.lista, 
                                    memoria->payload.catalogo.total_articulos, 
                                    carrito_local, 
                                    &total_carrito
                                );
                                if (comprar) {
                                    mostrar_ventana_emergente("Usa la seccion de carrito para procesar la orden");
                                }
                            } else {
                                mostrar_ventana_emergente(memoria->mensaje_alerta);
                            }
                        }
                        else if (opcion_sistema == 2) {
                            memoria->tipo = PETICION_VER_CARRITO;
                            
                            /* SEMAFOROS: Sincronismo de subida por canal 0 y retorno por canal 1 */
                            up_canal(semid_privado, 0);
                            
                            if (down_canal(semid_privado, 1) == -1) {
                                mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                cerrar_interfaz();
                                return 1;
                            }

                            int confirmar_compra = mostrar_carrito_compras(
                                carrito_local, 
                                total_carrito, 
                                memoria->payload.catalogo.lista, 
                                memoria->payload.catalogo.total_articulos
                            );

                            if (confirmar_compra) {
                                memoria->tipo = PETICION_COMPRAR_CARRITO;
                                memoria->payload.carrito.total_items = total_carrito;
                                memcpy(memoria->payload.carrito.items, carrito_local, sizeof(carrito_local));
                                
                                /* SEMAFOROS: Envio de la comanda por canal 0 y bloqueo de espera en canal 1 */
                                up_canal(semid_privado, 0);
                                
                                if (down_canal(semid_privado, 1) == -1) {
                                    mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                    cerrar_interfaz();
                                    return 1;
                                }

                                if (memoria->estado_respuesta == 1) {
                                    mostrar_ventana_emergente("Comanda enviada a barra exitosamente!");
                                    total_carrito = 0;
                                } else {
                                    mostrar_ventana_emergente(memoria->mensaje_alerta);
                                }
                            }
                        }
                        else if (opcion_sistema == 3) {
                            strcpy(memoria->payload.auth.usuario, usuario_autenticado);
                            memoria->tipo = PETICION_VER_PERFIL;
                            
                            /* SEMAFOROS: Solicitud de lectura de perfil en canal 0 y espera en canal 1 */
                            up_canal(semid_privado, 0);
                            
                            if (down_canal(semid_privado, 1) == -1) {
                                mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                cerrar_interfaz();
                                return 1;
                            }

                            char n_nom[MAX_TEXTO], n_ape[MAX_TEXTO], n_cor[MAX_TEXTO], n_pass[MAX_CONTRASENA] = "";
                            strcpy(n_nom, memoria->payload.auth.nombre);
                            strcpy(n_ape, memoria->payload.auth.apellido);
                            strcpy(n_cor, memoria->payload.auth.correo);

                            int guardar = formulario_perfil(n_nom, n_ape, n_cor, n_pass);
                            if (guardar) {
                                if (strlen(n_nom) == 0 || strlen(n_ape) == 0 || strlen(n_cor) == 0 || strlen(n_pass) == 0) {
                                    mostrar_ventana_emergente("Error: No se permiten campos vacios");
                                } else if (!validar_correo(n_cor)) {
                                    mostrar_ventana_emergente("Error: Estructura de correo invalida");
                                } else if (!validar_contrasena_segura(n_pass)) {
                                    mostrar_ventana_emergente("Error: La contraseña ingresada es insegura");
                                } else {
                                    strcpy(memoria->payload.auth.usuario, usuario_autenticado);
                                    strcpy(memoria->payload.auth.nombre, n_nom);
                                    strcpy(memoria->payload.auth.apellido, n_ape);
                                    strcpy(memoria->payload.auth.correo, n_cor);
                                    cifrar_contrasena(n_pass, memoria->payload.auth.contrasena_cifrada);
                                    memoria->tipo = PETICION_MODIFICAR_PERFIL;
                                    
                                    /* SEMAFOROS: Commit de alteracion de datos por canal 0 y retorno por canal 1 */
                                    up_canal(semid_privado, 0);
                                    
                                    if (down_canal(semid_privado, 1) == -1) {
                                        mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                                        cerrar_interfaz();
                                        return 1;
                                    }
                                    
                                    if (memoria->estado_respuesta == 1) {
                                        mostrar_ventana_emergente("Perfil actualizado correctamente");
                                    } else {
                                        mostrar_ventana_emergente(memoria->mensaje_alerta);
                                    }
                                }
                            }
                        }
                    }
                    break;
                } else {
                    mostrar_ventana_emergente(memoria->mensaje_alerta);
                    pass[0] = '\0';
                }
            }
        }
        else if (opcion_menu == 2) {
            char nom[MAX_TEXTO] = "", ape[MAX_TEXTO] = "", cor[MAX_TEXTO] = "", usr[MAX_USUARIO] = "", pass[MAX_CONTRASENA] = "";

            while (1) {
                int accion = formulario_registro(nom, ape, cor, usr, pass);
                if (accion == 0) {
                    break;
                }
                if (accion == 2) {
                    mostrar_requisitos_contrasena();
                    continue;
                }

                if (strlen(nom) == 0 || strlen(ape) == 0 || strlen(cor) == 0 || strlen(usr) == 0 || strlen(pass) == 0) {
                    mostrar_ventana_emergente("Error: Todos los campos son obligatorios");
                } else if (!validar_correo(cor)) {
                    mostrar_ventana_emergente("Error: Correo electronico invalido");
                } else if (!validar_contrasena_segura(pass)) {
                    mostrar_ventana_emergente("Error: Contraseña insegura");
                } else {
                    strcpy(memoria->payload.auth.usuario, usr);
                    strcpy(memoria->payload.auth.nombre, nom);
                    strcpy(memoria->payload.auth.apellido, ape);
                    strcpy(memoria->payload.auth.correo, cor);
                    cifrar_contrasena(pass, memoria->payload.auth.contrasena_cifrada);
                    memoria->tipo = PETICION_REGISTRO;

                    /* SEMAFOROS: Envio de registro de mesero en canal 0 y bloqueo de espera en canal 1 */
                    up_canal(semid_privado, 0);
                    
                    if (down_canal(semid_privado, 1) == -1) {
                        mostrar_ventana_emergente("Se ha perdido la conexion con el servidor");
                        cerrar_interfaz();
                        return 1;
                    }

                    if (memoria->estado_respuesta == 1) {
                        mostrar_ventana_emergente("Usuario registrado exitosamente!");
                        break;
                    } else {
                        mostrar_ventana_emergente(memoria->mensaje_alerta);
                    }
                }
            }
        }
    }

    shmdt(memoria);
    cerrar_interfaz();
    return 0;
}