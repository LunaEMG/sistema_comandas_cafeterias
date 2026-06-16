#ifndef COMUNES_H
#define COMUNES_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RUTA_BASE "."
#define ID_CANAL_PUBLICO 'P'
#define ID_MUTEX_PUBLICO 'M'

#define MAX_USUARIO 30
#define MAX_CONTRASENA 30
#define MAX_CIFRADO 61
#define MAX_TEXTO 50
#define MAX_ITEMS_CARRITO 20
#define MAX_PRODUCTOS_CATALOGO 50

typedef enum {
    PETICION_LOGIN,
    PETICION_REGISTRO,
    PETICION_VER_PRODUCTOS,
    PETICION_AGREGAR_CARRITO,
    PETICION_VER_CARRITO,
    PETICION_COMPRAR_CARRITO,
    PETICION_VER_PERFIL,
    PETICION_MODIFICAR_PERFIL,
    PETICION_ADMIN_LISTAR_USUARIOS,
    PETICION_ADMIN_MODIFICAR_USUARIO,
    PETICION_ADMIN_AGREGAR_PRODUCTO,
    PETICION_ADMIN_MODIFICAR_PRODUCTO,
    PETICION_ADMIN_REPORTE_VENTAS,
    PETICION_LOGOUT
} TipoPeticion;

typedef struct {
    char usuario[MAX_USUARIO];
    char contrasena_cifrada[MAX_CIFRADO];
    char nombre[MAX_TEXTO];
    char apellido[MAX_TEXTO];
    char correo[MAX_TEXTO];
} DatosAutenticacion;

typedef struct {
    int id_producto;
    char nombre[MAX_TEXTO];
    float precio;
    int stock;
} ProductoSHM;

typedef struct {
    int id_producto;
    int cantidad;
    float subtotal;
} ItemCarrito;

typedef struct {
    float total_diario;
    float total_semanal;
    float total_mensual;
    char desglose_reporte[2048];
} DatosReporte;

/* --- MEMORIA COMPARTIDA PRIVADA (CLIENTE <-> HILO SERVIDOR) --- */
typedef struct {
    pid_t pid_cliente;
    TipoPeticion tipo;
    int estado_respuesta;
    int latido_servidor;
    char mensaje_alerta[150];

    union {
        DatosAutenticacion auth;

        struct {
            ProductoSHM lista[MAX_PRODUCTOS_CATALOGO];
            int total_articulos;
        } catalogo;

        struct {
            ItemCarrito items[MAX_ITEMS_CARRITO];
            int total_items;
        } carrito;

        DatosReporte reportes;
    } payload;

} MemoriaPrivada;

typedef struct {
    pid_t pid_solicitante;
    key_t llave_shm_privada;
    key_t llave_sem_privado;
    int estatus;
} CanalPublico;

/* --- PRIMITIVAS NATIVAS DE SEMAFOROS (SYSTEM V) --- */

/* USO DE SEMAFOROS:
   Creacion de un conjunto de semaforos e inicializacion en el kernel */
static inline int crear_semaforo(key_t llave, int cantidad, int valor_inicial) {
    int semid = semget(llave, cantidad, IPC_CREAT | 0777);

    if (semid == -1) {
        semid = semget(llave, cantidad, 0777);

        if (semid == -1) {
            return -1;
        }
    }

    for (int i = 0; i < cantidad; i++) {
        semctl(semid, i, SETVAL, valor_inicial);
    }

    return semid;
}

/* USO DE SEMAFOROS:
   Operacion Wait / Decremento bloqueante apuntando a un indice del conjunto */
static inline int down_canal(int semid, int num_sem) {
    struct sembuf op_p[] = {
        {(unsigned short) num_sem, -1, 0}
    };

    return semop(semid, op_p, 1);
}

/* USO DE SEMAFOROS:
   Operacion Signal / Incremento de activacion apuntando a un indice del conjunto */
static inline int up_canal(int semid, int num_sem) {
    struct sembuf op_v[] = {
        {(unsigned short) num_sem, +1, 0}
    };

    return semop(semid, op_v, 1);
}

#endif