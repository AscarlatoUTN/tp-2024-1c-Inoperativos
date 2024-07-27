include settings.mk

#---------------------------- Definición de Variables ------------------------------

# Compilador
CC=gcc

# Archivadores
AR=ar
ARFLAGS=rcs

# Archivos Fuente (.c)
SRCS_C += $(shell find src/ -iname "*.c")  # Busca todos los archivos .c en el directorio src/

# Archivos de Enlace (.h) 
SRCS_H += $(shell find src/ -iname "*.h")  # Busca todos los archivos .h en el directorio src/

# Dependencias
DEPS = $(foreach STL,$(STATIC_LIBPATHS),$(STL:%=%/bin/lib$(notdir $(STL)).a))  # Genera las dependencias de bibliotecas estáticas

# Directorios de Enlace (-I)
IDIRS += $(addsuffix /src,$(STATIC_LIBPATHS) .)  # Rutas de encabezado

# Directorios de Enlace (-L)
LIBDIRS = $(addsuffix /bin,$(STATIC_LIBPATHS))     # Rutas de biblioteca

# Objetos intermedios generados a partir de los fuente
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS_C))  

# Ruta del Archivo de Salida
BIN = bin/lib$(shell cd . && pwd | xargs basename).a  

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

# Elimina los archivos generados
.PHONY: clean
clean:
	-rm -rfv obj bin  

#---------------------------- Reglas --------------------------------------

# de Construccion

# Construye el archivo binario final
$(BIN): $(OBJS) | $(dir $(BIN))
	$(AR) $(ARFLAGS) -o "$@" $^  # Archiva los objetos intermedios en el binario final

# Compila los archivos fuente en objetos intermedios
obj/%.o: src/%.c $(SRCS_H) $(DEPS) | $(dir $(OBJS))
	$(CC) $(CFLAGS) -fPIC -c -o "$@" $< $(IDIRS:%=-I%)  # Compila los archivos fuente en objetos intermedios

# de Dependencia

# Genera dependencias automáticas de archivos fuente y encabezados
.SECONDEXPANSION:
$(DEPS): $$(shell find $$(patsubst %bin/,%src/,$$(dir $$@)) -iname "*.c" -or -iname "*.h")
	make --no-print-directory -C $(patsubst %bin/,%,$(dir $@)) 3>&1 1>&2 2>&3 | sed -E 's,(src/)[^ ]+\.(c|h)\:,$(patsubst %bin/,%,$(dir $@))&,' 3>&2 2>&1 1>&3

# Crea los directorios necesarios para el binario y los objetos intermedios
$(sort $(dir $(BIN) $(OBJS))):
	mkdir -pv $@