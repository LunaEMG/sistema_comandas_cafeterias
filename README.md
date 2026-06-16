# ☕ Sistema de Comanda Digital (IPC)

Un sistema concurrente y distribuido de manera local, desarrollado en **C** para entornos **Linux**. Diseñado para optimizar la gestión operativa en cafeterías mediante la digitalización de la toma de órdenes, el control de inventario y la visualización de reportes en tiempo real.

Este proyecto fue desarrollado como parte de la asignatura de **Sistemas Operativos** (ESCOM - IPN) para demostrar el dominio de la comunicación entre procesos a bajo nivel.

## 🖼️ Preview del sistema

* **Servidor:**
  
  <img width="804" height="325" alt="Screenshot from 2026-06-15 22-30-14" src="https://github.com/user-attachments/assets/353eaecc-c4b1-482b-b5af-dc4e38fdb710" />

* **Cliente:**

<img width="869" height="624" alt="image" src="https://github.com/user-attachments/assets/9a5b0116-427e-40eb-824e-db4e342e8bc6" />


* **Administrador:**

<img width="869" height="624" alt="image" src="https://github.com/user-attachments/assets/425eea8d-3d78-4128-8c13-21323a281077" />

---

## ✨ Características Principales

* **Arquitectura Concurrente:** Implementación de un servidor central multihilo (`pthreads`) capaz de atender múltiples clientes simultáneamente.
* **Comunicación entre Procesos (IPC):** Sincronización robusta mediante **Semáforos** (System V) y transferencia de datos de latencia cero utilizando **Memoria Compartida**.
* **Interfaz de Usuario (TUI):** Interfaces dinámicas de terminal desarrolladas modularmente con la biblioteca `ncurses`.
* **Seguridad y Persistencia:** Cifrado XOR para el manejo de credenciales y persistencia de datos mediante archivos de texto plano (`usuarios.txt`, `articulos.txt`, `ventas.txt`).
* **Prevención de Deadlocks:** Manejo cuidadoso de la exclusión mutua para evitar condiciones de carrera (race conditions) al actualizar inventarios.

---

## 🛠️ Requisitos Previos

Para compilar y ejecutar este proyecto, necesitas un entorno Linux (nacional o mediante WSL en Windows) con las siguientes herramientas instaladas:

1. **Compilador GCC y Make** (Build essentials).
2. **Biblioteca Ncurses** (con soporte para caracteres anchos).

Si estás en una distribución basada en Debian/Ubuntu, puedes instalar los requisitos con los siguientes comandos:

```bash
sudo apt update
sudo apt install build-essential libncursesw5-dev libncurses5-dev

```

---

## 🚀 Compilación e Instalación

El proyecto incluye un archivo `Makefile` para facilitar su compilación.

1. Clona este repositorio en tu máquina local:

```bash
git clone https://github.com/LunaEMG/sistema_comandas_cafeterias.git

```

2. Navega al directorio del proyecto:

```bash
cd sistema_comandas_cafeterias

```

3. Compila todos los módulos ejecutando:

```bash
make

```

*(Este comando generará tres ejecutables: `servidor`, `cliente` y `administrador`).*

---

## 💻 Guía de Ejecución

Debido a la naturaleza cliente-servidor del sistema, necesitarás abrir **múltiples terminales** para probar su funcionamiento concurrente.

### Paso 1: Iniciar el Servidor Central

El servidor es el núcleo del sistema y **debe ejecutarse primero**. Éste inicializa los archivos del sistema, crea los segmentos de memoria compartida pública y levanta los semáforos centrales.
En tu primera terminal ejecuta:

```bash
./servidor

```

*(No cierres esta terminal. Verás un panel de logs en tiempo real monitoreando los TIDs y PIDs de las peticiones).*

### Paso 2: Iniciar el Panel de Administrador (Gerencia)

En una **segunda terminal**, ejecuta el panel gerencial. Este módulo permite registrar nuevos productos, ver reportes contables y el catálogo del personal.

```bash
./administrador

```

> **Nota:** En el primer arranque, el servidor genera un usuario administrador por defecto.
> * **Usuario:** `admin`
> * **Contraseña:** `Admin123!`
> 
> 

### Paso 3: Iniciar las Terminales de Cliente (Meseros)

Abre **una o más terminales adicionales** para simular a diferentes meseros operando al mismo tiempo. Aquí se podrán iniciar sesiones, ver el catálogo, y enviar comandas al servidor.

```bash
./cliente

```

---

## 🧹 Limpieza del Proyecto

Si deseas eliminar los archivos compilados (`.o`) y los ejecutables para recompilar el sistema desde cero, simplemente ejecuta:

```bash
make limpiar

```

---

## 👥 Equipo de Desarrollo

Proyecto desarrollado por alumnos de la carrera de Ingeniería en Sistemas Computacionales de la Escuela Superior de Cómputo (ESCOM - IPN):

* Delgado Guzmán Francisco Gabriel
* Luna Miguel Emmanuel
* Olascoaga Méndez Samadhi
* Sánchez Gutiérrez Fernando Uriel

**Director / Asesor:** Dr. Jiménez Benítez José Alfredo
