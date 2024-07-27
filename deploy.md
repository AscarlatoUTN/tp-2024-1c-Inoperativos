## TOKENS DEL GRUPO

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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
    

# COMANDOS DEPLOY GENERAL 

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Clonar el script de despliegue
    git clone https://github.com/sisoputnfrba/so-deploy.git

#### 3. Ingresar al directorio
    cd so-deploy

#### 4. Ejecutar el script
    ./deploy.sh -r=release -p=SHARED -p=MEMORIA -p=CPU -p=KERNEL -p=IO tp-2024-1c-Inoperativos

#### 5. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

#### 6. Llenar archivos de config con la IP y el Puerto de cada módulo.

    ./configure.sh IP_MEMORIA x
    ./configure.sh IP_CPU x
    ./configure.sh IP_KERNEL x
    ./configure.sh PUERTO_KERNEL x
    ./configure.sh PUERTO_MEMORIA x
    ./configure.sh PUERTO_ESCUCHA_DISPATCH x
    ./configure.sh PUERTO_ESCUCHA_INTERRUPT x
    ./configure.sh PUERTO_ESCUCHA x

.
.
.
.
.
.
.
.

# COMANDOS POR PRUEBA

## PRUEBA PLANIFICACION

### COMANDOS DEPLOY

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Clonar el script de despliegue
    git clone https://github.com/sisoputnfrba/so-deploy.git

#### 3. Ingresar al directorio
    cd so-deploy

#### 4. Ejecutar el script
    ./deploy.sh -r=release -p=SHARED -p=MEMORIA -p=CPU -p=KERNEL -p=IO tp-2024-1c-Inoperativos

#### 5. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

#### 6. Llenar archivos de config con la IP y el Puerto de cada módulo.

    ./configure.sh IP_MEMORIA x
    ./configure.sh IP_CPU x
    ./configure.sh IP_KERNEL x
    ./configure.sh PUERTO_KERNEL x
    ./configure.sh PUERTO_MEMORIA x
    ./configure.sh PUERTO_ESCUCHA_DISPATCH x
    ./configure.sh PUERTO_ESCUCHA_INTERRUPT x
    ./configure.sh PUERTO_ESCUCHA x

### Pasos

#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_plani.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_plani.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_plani.config 

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    ./bin/IO SLP1 CONFIGS/PRUEBA_PLANI/SLP1.config

#### 5. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_PLANI

#### 6. KERNEL

***Cuando empiece a ejecutar PLANI_4***
    
    FINALIZAR_PROCESO 4

***Esperar a que finalicen los demás procesos y finalizar todos los modulos*** 

***REVISAR***
- 3 de los 4 procesos finalizan sin problemas
- Para que puedan volver a ejecutar PLANI_2 y PLANI_3 hay que matar el proceso PLANI_4

#### 7. KERNEL
    sed -i 's/^ALGORITMO_PLANIFICACION=.*/ALGORITMO_PLANIFICACION=RR/' k_plani.config

#### 8. MEMORIA
    ./bin/MEMORIA m_plani.config

#### 9. CPU
    ./bin/CPU c_plani.config

#### 10. KERNEL          
    ./bin/KERNEL k_plani.config 

#### 11. IO
    ./bin/IO SLP1 CONFIGS/PRUEBA_PLANI/SLP1.config

#### 12. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_PLANI

#### 13. KERNEL
***Esperar a que finalicen los 3 procesos***
    
    FINALIZAR_PROCESO 4

***Finalizar la ejecución***

***REVISAR***
- **Orden de Finalización**: PLANI_1, PLANI_3, PLANI_2. 
- PLANI_4 puede segur ejecutandose, porque es un loop. 
- PLANI_3 es desalojado 2 veces por fin de quantum.


#### 14. KERNEL
    sed -i 's/^ALGORITMO_PLANIFICACION=.*/ALGORITMO_PLANIFICACION=VRR/' k_plani.config

#### 15. MEMORIA
    ./bin/MEMORIA m_plani.config

#### 16. CPU
    ./bin/CPU c_plani.config

#### 17. KERNEL          
    ./bin/KERNEL k_plani.config 

#### 18. IO
    ./bin/IO SLP1 CONFIGS/PRUEBA_PLANI/SLP1.config

#### 19. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_PLANI

***Esperar a que finalicen los 3 procesos y cortar la prueba***

***REVISAR***
- **Orden de Finalización**: PLANI_1, PLANI_3, PLANI_2. 
- PLANI_4 puede segur ejecutandose, porque es un loop. 
- PLANI_3 es desalojado 3 veces por fin de quantum.


### Resultados Esperados:

- 3 de los 4 procesos finalizan sin problemas
- En FIFO Para que puedan volver a ejecutar PLANI_2 y PLANI_3 hay que matar el proceso PLANI_4
- En RR finalizan PLANI_1, luego PLANI_3 (el cual es desalojado 2 veces por fin de quantum) y por último PLANI_2. PLANI_4 continúa ejecutando.
- En VRR finalizan en el mismo orden que RR, pero PLANI_3 es desalojado 3 veces por fin de quantum.


## PRUEBA DEADLOCK

### COMANDOS DEPLOY

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Ingresar al directorio
    cd so-deploy

#### 3. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

### Pasos

#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_deadlock.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_deadlock.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_deadlock.config

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    ./bin/IO ESPERA CONFIGS/PRUEBA_PLANI/ESPERA.config

#### 5. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_DEADLOCK

#### 6. KERNEL

***Cuando los 4 procesos entren en deadlock***

    FINALIZAR_PROCESO 4

***Esperar a que finalice el resto de procesos***

### Resultados Esperados
    - Los 4 procesos se quedan en deadlock
    - Al finalizar un proceso los demás se liberan

## PRUEBA MEMORIA Y TLB

#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_tlb.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_tlb.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_tlb.config

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    ./bin/IO IO_GEN_SLEEP CONFIGS/PRUEBA_PLANI/IO_GEN_SLEEP.config

#### 5. KERNEL
    
    INICIAR_PROCESO scripts_memoria/MEMORIA_1

***Esperar a que finalice el proceso, y finlizar todos los modulos***

***REVISAR***
- Anotar la cantidad y orden de los TLB MISS y TLB HIT

#### 6. CPU
    sed -i 's/^ALGORITMO_TLB=.*/ALGORITMO_TLB=LRU/' c_tlb.config

#### 7. MEMORIA
    ./bin/MEMORIA m_tlb.config

#### 8. CPU
    ./bin/CPU c_tlb.config

#### 9. KERNEL
    ./bin/KERNEL k_tlb.config

#### 10. IO
    ./bin/IO IO_GEN_SLEEP CONFIGS/PRUEBA_PLANI/IO_GEN_SLEEP.config

#### 11. KERNEL
    
    INICIAR_PROCESO scripts_memoria/MEMORIA_1

***Esperar a que finalice el proceso.***

***REVISAR***
- Comparar la cantidad y orden de los TLB MISS y TLB HIT de esta ejecución con la anterior

#### 12. KERNEL
##### Esperar a que finalice el proceso
    
    INICIAR_PROCESO scripts_memoria/MEMORIA_2

***Esperar a que finalice el proceso.***
#### 13. KERNEL
##### Esperar a que finalice el proceso
    
    INICIAR_PROCESO scripts_memoria/MEMORIA_3

***Esperar a que finalice el proceso, terminar la prueba***

***REVISAR***
- MEMORIA_2 puede recuperar correctamente el valor de la memoria que se encuentra en 2 págs diferentes. 
- MEMORIA_3 es finalizado por Out Of Memory

### Resultados Esperados:

- En las ejecuciones del proceso MEMORIA_1 se observan diferencias en los reemplazos de la TLB.
- En la ejecución del proceso MEMORIA_2 se observa que puede recuperar correctamente el valor de la memoria que se encuentra en 2 páginas diferentes.
- En la ejecución del proceso MEMORIA_3 el mismo debería finalizar por error de Out of Memory.
## PRUEBA IO

### COMANDOS DEPLOY

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Ingresar al directorio
    cd so-deploy

#### 3. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

### Pasos
#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_io.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_io.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_io.config

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    ./bin/IO GENERICA CONFIGS/PRUEBA_IO/GENERICA.config
    
    ./bin/IO TECLADO CONFIGS/PRUEBA_IO/TECLADO.config
    
    ./bin/IO MONITOR CONFIGS/PRUEBA_IO/MONITOR.config

#### 5. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_IO

#### 6. IO
##### PARA IO_A:
    WAR NEVER CHANGES...

##### PARA IO_C:
    Sistemas Operativos 2c2023

***Esperar a que finalicen 3 procesos y cortar la prueba.***
### Resultados Esperados:

- El proceso IO_A debería imprimir la frase: “WAR, WAR NEVER CHANGES…”
- El proceso IO_B debería imprimir la frase: “I don't want to set the world on fire”
- El proceso IO_C debería imprimir la frase: “Sistemas Operativos 1c2024”


## PRUEBA FS

### COMANDOS DEPLOY

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Ingresar al directorio
    cd so-deploy

#### 3. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

### Pasos
#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_fs.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_fs.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_fs.config

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    ./bin/IO FS CONFIGS/PRUEBA_FS/FS.config

    ./bin/IO MONITOR CONFIGS/PRUEBA_FS/MONITOR.config

    ./bin/IO TECLADO CONFIGS/PRUEBA_FS/TECLADO.config

#### 5. Kernel

***Ejecutar los dos comandos de forma seguida.***

    INICIAR_PROCESO scripts_memoria/FS_1

    INICIAR_PROCESO scripts_memoria/FS_2

#### 6. IO
##### PARA FS_1:
    Fallout 1 Fallout 2 Fallout 3 Fallout: New Vegas Fallout 4 Fallout 76

***Finalizan los 2 procesos y finalizar todos los módulos.***

***REVISAR***
- FS_1 genera 2 archivos y escribe en ellos. 
- FS_2 genera 2 archivos de 10 bytes cada uno. 
#### 7. MEMORIA
    ./bin/MEMORIA m_fs.config

#### 8. CPU
    ./bin/CPU c_fs.config

#### 9. KERNEL
    ./bin/KERNEL k_fs.config

#### 10. IO
    ./bin/IO FS CONFIGS/PRUEBA_FS/FS.config

    ./bin/IO MONITOR CONFIGS/PRUEBA_FS/MONITOR.config

    ./bin/IO TECLADO CONFIGS/PRUEBA_FS/TECLADO.config

#### 11. Kernel

***Ejecutar los dos comandos de forma seguida.***

    INICIAR_PROCESO scripts_memoria/FS_3

    INICIAR_PROCESO scripts_memoria/FS_4

***Esperar a que finalicen los procesos y cortar la prueba.***

***REVISAR***
- FS_3 lee el contenido del primer archivo creado por FS_1
- FS_4 crea un archivo de 250 bytes, elimina el primer archivo creado por FS_2 y al ampliar el segundo archivo de los creados por FS_2 requiere compactar. 
### Resultados Esperados:

- El proceso FS_1 genera 2 archivos y escribe en ellos.
- El proceso FS_2 genera 2 archivos de tamaño 10 bytes.
- El proceso FS_3 lee el contenido del primer archivo creado por FS_1.
- El proceso FS_4 crea un archivo de 250 bytes, elimina el primer archivo creado por FS_2 y al querer ampliar el segundo archivo de los creados por FS_2 requiere de compactar.

## PRUEBA SALVATION´S EDGE

### COMANDOS DEPLOY

#### 1. Conectarse por SSH usando PuTTY. Se debe usar el puerto 22.

#### 2. Ingresar al directorio
    cd so-deploy

#### 3. Ingresar el Personal Access Token de GitHub cuando se lo pida

**ALE** 
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
    
**TEI** 
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
    
**ELI** 
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
    
**NATI** 
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
    
**COLO** 
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

### Pasos
#### 1. MEMORIA
    cd tp-2024-1c-Inoperativos/MEMORIA
    ./bin/MEMORIA m_salvation.config

#### 2. CPU
    cd tp-2024-1c-Inoperativos/CPU
    ./bin/CPU c_salvation.config

#### 3. KERNEL
    cd tp-2024-1c-Inoperativos/KERNEL
    ./bin/KERNEL k_salvation.config

#### 4. IO
    cd tp-2024-1c-Inoperativos/IO
    
    ./bin/IO GENERICA CONFIGS/PRUEBA_SALVATION/GENERICA.config
    
    ./bin/IO ESPERA CONFIGS/PRUEBA_SALVATION/ESPERA.config
    
    ./bin/IO TECLADO CONFIGS/PRUEBA_SALVATION/TECLADO.config
    
    ./bin/IO SLP1 CONFIGS/PRUEBA_SALVATION/SLP1.config
    
    ./bin/IO MONITOR CONFIGS/PRUEBA_SALVATION/MONITOR.config

#### 5. KERNEL
    EJECUTAR_SCRIPT scripts_kernel/PRUEBA_SALVATIONS_EDGE

#### 6. Kernel

***Esperar a que empiecen a ejecutar los primeros dos scripts***
    
    MULTIPROGRAMACION 100

#### 7. Kernel
###### A intervalos regulares ejecutar los siguientes comandos en la consola del Kernel
   
    DETENER_PLANIFICACION
    
    PROCESO_ESTADO
    
    INICIAR_PLANIFICACION

### Resultados Esperados: 

- No se observan esperas activas y/o memory leaks.
