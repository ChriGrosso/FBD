# Definimos el compilador que se usará (gcc) y habilitamos la generación de información de depuración (-g)
CC = gcc -g
# Especificamos las banderas para el compilador:
# -Wall: Activa todos los avisos del compilador
# -Wextra: Activa advertencias adicionales
# -pedantic: Fuerza el cumplimiento estricto del estándar
# -ansi: Restringe el código al estándar ANSI C
CFLAGS = -Wall -Wextra -pedantic -ansi

# Archivos de cabecera necesarios para la recompilación si cambian
HEADERS = utils.h

# Nombre del ejecutable
EXE = tester
# Archivos objeto necesarios para construir el ejecutable
OBJ = $(EXE).o utils.o checkUtils.o checkCreateTable.o checkReplaceExtensionByIdx.o \
checkCreateIndex.o checkPrint.o checkFindKey.o checkAddIndexEntry.o
#all : $(EXE)

# Regla implícita para compilar archivos .c a .o usando el compilador y las banderas definidas
%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c -o $@ $<

# Regla para construir el ejecutable.
$EXE: $(DEPS) $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDLIBS)

# Regla para limpiar archivos generados
clean :
	rm -f *.o core $(EXE) 


