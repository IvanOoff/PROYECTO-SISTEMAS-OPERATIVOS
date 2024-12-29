#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#define LONGITUD_COMANDO 100

// Declaración de las funciones utilizadas en el programa.
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
int BuscaFich2(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombredestino);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

// ------------Main------------
int main() {
 
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

    // Abrimos el archivo con un "fopen".
    FILE *fent = fopen("particion.bin", "rb"); // Si se pone r+b el fichero cambiará su contenido permanentemente.
    if (!fent) {
        printf("ERROR: No se pudo abrir el archivo (particion.bin)\n");
        return -1;
    }

    // Leemos el contenido del archivo particion.bin con "fread", para después ir sacando la información de este mismo.
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Utilizamos un buble for que actuará como si se tratase de un "while(1)", donde se llamará a los comando.
    for(;;){
        printf(">> ");
        fgets(comando, LONGITUD_COMANDO, stdin);
        comando[strcspn(comando, "\n")] = 0;

        // Verificamos que el usaurio haya introducido bien el comando.
        if (ComprobarComando(comando, orden, argumento1, argumento2) != 0) {
            printf("ERROR: Comando invalido. Intente nuevamente.\n"); // Saltara un error en caso de que el usuario introduzca un comando erroneo.
            continue;
        }

        // ---INFO---
        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        }

        // ---BYTEMAPS---
        else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } 

        // ---DIR---
        else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        }

        // ---RENAME---
        else if (strcmp(orden, "rename") == 0) {
            if (Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == 0) {
                printf("Fichero renombrado con exito.\n");
                
                // Aseguramos que se guardan los cambios en memoria.
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
        } 
        
        // ---REMOVE---
        else if (strcmp(orden, "remove") == 0) {
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == 0) {
                printf("Fichero eliminado con exito.\n");
                
                // Aseguramos que se guardan los cambios en memoria.
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
        } 

        // ---COPY---
        else if (strcmp(orden, "copy") == 0) {
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == 0) {
                printf("Fichero copiado con exito.\n");
                
                // Aseguramos que se guardan los cambios en memoria.
                Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
                GrabarByteMaps(&ext_bytemaps, fent);
                GrabarSuperBloque(&ext_superblock, fent);
            }
        }
        // ---IMPRIMIR---
        else if (strcmp(orden, "imprimir") == 0) {
            if (Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1) == 0) {
                printf("Contenido impreso con exito.\n");
            }
        }

        // ---SALIR---
        else if (strcmp(orden, "salir") == 0) {
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            GrabarByteMaps(&ext_bytemaps, fent);
            GrabarSuperBloque(&ext_superblock, fent);
            GrabarDatos(memdatos, fent);

            // Lo cerramos.
            fclose(fent);
            printf("Saliendo del sistema de archivos....\n");
            break;
        } 

        // ---COMANDO DESCONOCIDO---
        else {
            printf("ERROR: Comando ilegal [bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
        }
    }
    return 0;
}

// ---------------MÉTODOS---------------


// ---INFO--- / Método que mostrará la información del superbloque por la consola.
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {

    // Imprimimos la información del superbloque.
    printf("Bloques %d bytes\n", psup->s_block_size);
    printf("Inodos particion = %d\n", psup->s_inodes_count);
    printf("Inodos libres = %d\n", psup->s_free_inodes_count);
    printf("Bloques particion = %d\n", psup->s_blocks_count);
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

// ---BYTEMAPS--- / Método que mostrará el contenido del "bytemaps" de inodos y los 25 primeros elementos del bytemap de bloques.
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");

    // Bucle "for" que recogerá el contenido del bytemap de inodos.
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
        }
        
        printf("\nBloques [0-25]: ");
        
    // Bucle "for" que recogerá los 25 primeros elementos del bytemap de bloques.
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

// ---COMPROBAR COMANDO--- / Método encargado de comprobar si el usuario ha introducido el comando correctamente.
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    
    // Usamos "sscanf" para extraer y analizar "Strings" sacados de "strcomando", los cuales serán la orden y el argumento 1 y 2.
    if (sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2) >= 1) {
        return 0;
        }
        else{
            return -1;
        }
}

// ---BUSCARFICH--- / Método que se encargará de buscar el fichero.
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    
    // Bucle for para recorrer todas las entradas del directorio.
    for (int i = 0; i < MAX_FICHEROS; i++) {

        // Comprobamos si la entrada del directorio es válida y comparamos el nombre del archivo que se está buscando con el nombre guardado en el directorio actual.
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return directorio[i].dir_inodo; // Devolvemos el número de inodo del fichero encontrado.
        }
    }
    return -1; // Si no encuentra el directorio, o el nombre no coincide, el archivo no ha sido encontrado.
}

// ---BUSCARFICH--- / Método que se encargará de buscar el fichero / Para gestionar el método de rename y remove.
int BuscaFich2(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
        
    // Bucle for para recorrer todas las entradas del directorio.
    for (int i = 0; i < MAX_FICHEROS; i++) {

        // Comprobamos si la entrada del directorio es válida y comparamos el nombre del archivo que se está buscando con el nombre guardado en el directorio actual.
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i; // Devuelve el índice del directorio del fichero encontrado.
        }
    }
    return -1; // Si no encuentra el directorio, o el nombre no coincide, el archivo no ha sido encontrado.
}

// ---DIR--- / Método que imprime el contenido del directorio, el tamaño, inodo, nombre y bloques que ocupa.
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {

    // Bucle for para recorrer las entradas del directorio y empenzamos en 1 para que empieze a imprimir desde el primer archivo, ya que en 0 no hay nada.
    for (int i = 1; i < MAX_FICHEROS; i++) {

        // Comprobamos si la entrada del directorio está ya asignada.
        if (directorio[i].dir_inodo != NULL_INODO) {

            // Imprimimos el tamaño, inodo y bloques de todos los archivos del directorio.
            printf("%s tamanio:%d inodo:%d bloques:", directorio[i].dir_nfich, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, directorio[i].dir_inodo);
            

            // ----PARA LOS BLOQUES-----


            // Bucle para recorrer los bloques asignados al archivo indicado por el inodo.
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {

                // Comprobamos si el bloque asctual está asignado.
                if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE) {
                    printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
                }
            }   
            printf("\n"); // Salto de linea.
        }
    }
}

// ---RENAME--- / Método que cambiará el nombre del fichero en la entrada correspondiente.
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    
    // Llamamos a la función de "BuscaFich2" para encontrar el índice del archivo con el nombre antiguo en el directorio, y lo guardamos en "inodo_a_buscar".
    int inodo_a_buscar = BuscaFich2(directorio, inodos, nombreantiguo);
    
    // Comprobamos si el indice es "-1" para verificar que el archivo con el nombre antiguo no exista.
    if (inodo_a_buscar == -1) {
        printf("Error: el fichero %s no existe.\n", nombreantiguo);
        return -1;
    }

    // Comprobamos con "BuscarFich" si ya existe un archivo con el nombre nuevo del fichero.
    if (BuscaFich2(directorio, inodos, nombrenuevo) != -1) {
        printf("Error: el fichero %s ya existe.\n", nombrenuevo);
        return -1;
    }

    // Copiamos el nuevo nombre en la entrada del directorio correspondiente al inodo encontrado.
    strcpy(directorio[inodo_a_buscar].dir_nfich, nombrenuevo);
    return 0;
}

// ---IMPRIMIR--- / Método que mostrará el contenido del fichero.
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    
    // Llamamos a la función de "BucarFich" para encontrar el inodo que le corresponde al archivo con el nombre especificado y lo guardamos en la variable "inodo".
    int inodo = BuscaFich(directorio, inodos, nombre);
    
    // Comprobamos si el fichero existe.
    if (inodo == -1) {
        printf("Error: el fichero %s no existe.\n", nombre);
        return -1;
    }

    // Bucle for para recorrer todos y cada unos de los bloques de datos.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {

        // Comprobamos si el bloque actual no es nulo "NULL_BLOQUE".
        if (inodos->blq_inodos[inodo].i_nbloque[i] != NULL_BLOQUE) {
            printf("%s", memdatos[inodos->blq_inodos[inodo].i_nbloque[i] - PRIM_BLOQUE_DATOS].dato);
        }
    }
    printf("\n"); // Salto de linea.
    return 0;
}

// ---REMOVE--- / Método que borrará el fichero que le indiquemos.
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    
    // Llamamos a la función de "BuscaFich2" para encontrar el índice del archivo y lo guardamos en "indice".
    int inodo = BuscaFich2(directorio, inodos, nombre);
   
    // Si el indice guardado anteriormente es "-1", imprimimos un mensaje de "ERROR" indicando que el archivo no existe.
    if (inodo == -1) {
        printf("Error: el fichero %s no existe.\n", nombre);
        return -1;
    }

    // Bucle para recorrer los bloques asignados al inodo encontrado.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {

        // Comprobamos si un bloque no es "NULL_BLOQUE", lo cual significará qie el bloque está ya asignado.
        if (inodos->blq_inodos[inodo].i_nbloque[i] != NULL_BLOQUE) {

            // Marcamos ese bloque en el mapa de bits de bloques como libre "0".
            ext_bytemaps->bmap_bloques[inodos->blq_inodos[inodo].i_nbloque[i]] = 0;

            // Establecemos el bloque del inodo a "NULL_BLOQUE".
            inodos->blq_inodos[inodo].i_nbloque[i] = NULL_BLOQUE;
        }
    }

    // Establecemos el tamaño del archivo del inodo a 0.
    inodos->blq_inodos[inodo].size_fichero = 0;

    // Marcamos el inodo en el mapa de bits de inodos como libre "0".
    ext_bytemaps->bmap_inodos[inodo] = 0;

    // Establecemos el nombre del archivo en la entrada del directorio correspondiente al inodo.
    strcpy(directorio[inodo].dir_nfich, "");

    // Establecemos el inodo de la entrada del directorio a "NULL_INODO".
    directorio[inodo].dir_inodo = NULL_INODO;

    // Incrementamos el contador de bloques libres.
    ext_superblock->s_free_blocks_count++;

    // Incrementamos el contados de los inodos libres.
    ext_superblock->s_free_inodes_count++;
    return 0;
}

// ---COPY--- / Método que copiará el fichero en uno nuevo.
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    
    // Llamamos a la función de "BuscaFich" para encontrar el índice del archivo y lo guardamos en "inodo_origen".
    int inodo_origen = BuscaFich(directorio, inodos, nombreorigen);
    
    // Comprobamos si el fichero de origen introducido por el usuario existe.
    if (inodo_origen == -1) {
        printf("Error: el fichero %s no existe.\n", nombreorigen);
        return -1;
    }

    // Comprobamos si ya existe un archivo con el nombre del archivo de destino al que el usuario quiere mandar la copia del contenido del archivo de origen.
    if (BuscaFich(directorio, inodos, nombredestino) != -1) {
        printf("Error: el fichero %s ya existe.\n", nombredestino);
        return -1;
    }

    // Inicializamos el inodo de destino a -1.
    int inodo_destino = -1;

    // Bucle para recorrer todos los inodos y buscar uno que esté libre.
    for (int i = 0; i < MAX_INODOS; i++) {

        // Comprobamos si el inodo está libre -> 0.
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            inodo_destino = i;
            break;
        }
    }

    // Comprobamos si no se ha encontrado un inodo libre.
    if (inodo_destino == -1) {
        printf("Error: no hay inodos disponibles.\n");
        return -1;
    }

    // Marcamos el bloque actual como "ocupado" en el mapa de bits de bloques.
    ext_bytemaps->bmap_inodos[inodo_destino] = 1;

    // Asignamos el bloque actual al inodo de destino.
    inodos->blq_inodos[inodo_destino].size_fichero = inodos->blq_inodos[inodo_origen].size_fichero;
    
    // Bucle for para recorrer todos los bloques de daros referenciados por el inodo del archivo de origen.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        
        // Comprobamos si el bloque actual no es nulo "NULL_BLOQUE".
        if (inodos->blq_inodos[inodo_origen].i_nbloque[i] != NULL_BLOQUE) {
            
            // Bucle for que recorrerá el mapa de bits de bloques para encontrar un bloque libre.
            for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
                
                // Lo primero, comprobamos si el bloque actuale está libre -> 0.
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    
                    // En suegundo lugar, lo marcamos como "ocupado".
                    ext_bytemaps->bmap_bloques[j] = 1;
                    
                    // Asignamos el bloque actual al inodo de destino.
                    inodos->blq_inodos[inodo_destino].i_nbloque[i] = j;
                    
                    // Usamos "memcpy" para copiar el contenido del bloque de datos del inodo de origen al bloque de datos del inodo de destino.
                    memcpy(memdatos[j - PRIM_BLOQUE_DATOS].dato, memdatos[inodos->blq_inodos[inodo_origen].i_nbloque[i] - PRIM_BLOQUE_DATOS].dato, SIZE_BLOQUE);
                    break;
                }
            }
        } else {
            // Sino, marcamos el bloque del inodo de destino como nulo.
            inodos->blq_inodos[inodo_destino].i_nbloque[i] = NULL_BLOQUE;
        }
    }

    // Bucle para recorrer todas las entradas del directorio para buscar una entrada libre.
    for (int i = 0; i < MAX_FICHEROS; i++) {

        // Comprobamos si la entrada actual está libre o es "NULL_INODO".
        if (directorio[i].dir_inodo == NULL_INODO) {
            
            // Copiamos el nombre del archivo de destino usando "strcpy" a la entrada libre del directorio.
            strcpy(directorio[i].dir_nfich, nombredestino);
            
            // Asignamos el inodo destino a la entrada del directorio.
            directorio[i].dir_inodo = inodo_destino;
            break;
        }
    }
    
    // Decrementamos el contados de los bloques libres.
    ext_superblock->s_free_blocks_count--;
    
    // Decrementamos el contador de inodos libres.
    ext_superblock->s_free_inodes_count--;
    return 0;
}

// ---------------GUARDADO DE DATOS---------------

// ---GRABAR INODOS Y DIRECTORIO---- / Método para guardar los inodos y el directorio.
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo hasta la posición que le corresponda.
    fseek(fich, 3 * SIZE_BLOQUE, SEEK_SET);

    // Escribimos con "fwrite" una copia de la estructura de inodos en el archivo, y le indicamos que ocupe solo un bloque.
    fwrite(directorio, SIZE_BLOQUE, 1, fich);

    // Usamos "fseek" para desplazar el puntero del archivo hasta la posición que le corresponda.
    fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);

    // Escribimos con "fwrite" una copia de la estructura de inodos en el archivo, y le indicamos que ocupe solo un bloque.
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR BYREMAPS---- / Método para guardar los mapas de bits.

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo al segundo bloque del archivo, que es donde se almacenan los mapas de bits.
    fseek(fich, 1 * SIZE_BLOQUE, SEEK_SET);

    // Con "fwrite" escribimos los mapas de bits en ese mismo bloque.   
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR SUPERBLOQUE---- / Método para guardar el superbloque.
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo al principio del archivo o primer bloque.
    fseek(fich, 0 * SIZE_BLOQUE, SEEK_SET);
    
    // Con "fwite" escribimos la estructura del superbloque en el primer bloque.
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR DATOS---- / Método para guardar los datos.
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    // Usamos "fseek" para desplazar el puntero del archivo hasta el cuato bloque del archivo, que es donde inicia el área de datos.
    fseek(fich, 4 * SIZE_BLOQUE, SEEK_SET);

    // Escribimos con "fwrite" todos los bloques de datos desde el cuarto bloque (donde inicia el área de datos) en adelante
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}