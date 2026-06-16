compilador_c = gcc
banderas_c = -Wall -Wextra -std=c99

objetos_seguridad = cifrado.o validacion.o
objetos_interfaz = interfaz_comun.o interfaz_cliente.o interfaz_admin.o

todo: servidor cliente administrador

servidor: servidor.o cifrado.o
	$(compilador_c) $(banderas_c) servidor.o cifrado.o -o servidor -lpthread

cliente: cliente.o $(objetos_seguridad) $(objetos_interfaz)
	$(compilador_c) $(banderas_c) cliente.o $(objetos_seguridad) $(objetos_interfaz) -o cliente -lncursesw

administrador: administrador.o $(objetos_seguridad) $(objetos_interfaz)
	$(compilador_c) $(banderas_c) administrador.o $(objetos_seguridad) $(objetos_interfaz) -o administrador -lncursesw

%.o: %.c comunes.h
	$(compilador_c) $(banderas_c) -c $< -o $@

limpiar:
	rm -f *.o servidor cliente administrador