# ☕ Sistema de Comanda Digital (IPC)

Un sistema concurrente y distribuido de manera local, desarrollado en **C** para entornos **Linux**. Diseñado para optimizar la gestión operativa en cafeterías mediante la digitalización de la toma de órdenes, el control de inventario y la visualización de reportes en tiempo real.

Este proyecto fue desarrollado como parte de la asignatura de **Sistemas Operativos** (ESCOM - IPN) para demostrar el dominio de la comunicación entre procesos a bajo nivel.

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

Si estás en una distribución basada en Debian/Ubuntu, puedes instalar los requisitos con el siguiente comando:
```bash
sudo apt update
sudo apt install build-essential libncursesw5-dev libncurses5-dev
