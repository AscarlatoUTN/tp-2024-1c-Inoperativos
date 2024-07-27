include settings.mk

#---------------------------- Definición de Variables ------------------------------

# Compilador
CC=gcc

# Archivos Fuente (.c)
SRCS_C += main.c                                  # Agrega el archivo main.c ubicado fuera del directorio src
SRCS_C += $(shell find src/ -iname "*.c")         # Agrega los archivos .c encontrados en el directorio src

# Archivos de Enlace (.h) 
SRCS_H += $(shell find include/ -iname "*.h")

# Dependencias
DEPS = $(foreach STL,$(STATIC_LIBPATHS),$(STL:%=%/bin/lib$(notdir $(STL)).a))

# Directorios de Enlace (-I)
IDIRS += $(addsuffix /src,$(STATIC_LIBPATHS) .)

# Directorios de Enlace (-L)
LIBDIRS = $(addsuffix /bin,$(STATIC_LIBPATHS))

# Objetos intermedios generados a partir de los fuente
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS_C))

# Ruta del Archivo de Salida
BIN = bin/$(shell cd . && pwd | xargs basename)

#---------------------------- Targets Phony --------------------------------------

# Establece debug como el objetivo predeterminado
.PHONY: all
all: debug

# Modo de depuración
.PHONY: debug
debug: CFLAGS = $(CDEBUG)
debug: $(BIN)

# Modo de liberación
.PHONY: release
release: CFLAGS = $(CRELEASE)
release: $(BIN)

# Limpia los archivos generados
.PHONY: clean
clean:
	-rm -rfv obj bin

# Limpia los archivos de log
.PHONY: logs
logs:
	rm -f *.log

# Ejecuta el proyecto en modo debug con valgrind
.PHONY: start
start: debug
	valgrind --tool=none ./$(BIN) $(ARGS)

# Ejecuta valgrind para comprobar fugas de memoria
.PHONY: valgrind
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=definite,indirect --track-origins=yes --log-file=valgrind.log $(MEMCHECK_FLAGS) ./$(BIN) $(ARGS)

# Ejecuta valgrind con helgrind para detectar problemas de concurrencia
.PHONY: helgrind
helgrind: debug
	valgrind --tool=helgrind --log-file=valgrind.log $(HELGRIND_FLAGS) ./$(BIN) $(ARGS)

#---------------------------- Reglas --------------------------------------

# de Construccion

# Construye el archivo binario final
$(BIN): $(OBJS) | $(dir $(BIN))
	$(CC) $(CFLAGS) -o "$@" $^ $(IDIRS:%=-I%) $(LIBDIRS:%=-L%) $(LIBS:%=-l%)

# Compila los archivos fuente en objetos intermedios
obj/%.o: src/%.c $(SRCS_H) $(DEPS) | $(dir $(OBJS))
	$(CC) $(CFLAGS) -c -o "$@" $< $(IDIRS:%=-I%)

# de Dependencia

# Genera dependencias automáticas de archivos fuente y encabezados
.SECONDEXPANSION:
$(DEPS): $$(shell find $$(patsubst %bin/,%src/,$$(dir $$@)) -iname "*.c" -or -iname "*.h")
make --no-print-directory -C $(patsubst %bin/,%,$(dir $@)) 3>&1 1>&2 2>&3 | sed -E 's,(src/)[^ ]+\\.(c|h)\\:,$(patsubst %bin/,%,$(dir $@))&,' 3>&2 2>&1 1>&3

# Adicionales

# Crea directorios si no existen
$(sort $(dir $(BIN) $(OBJS))):
	mkdir -pv $@