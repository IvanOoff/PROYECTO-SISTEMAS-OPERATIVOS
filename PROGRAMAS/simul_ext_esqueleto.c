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
    FILE *fent = fopen("particion.bin", "r+");
    if (!fent) {
        printf("ERROR: No se pudo abrir el archivo particion.bin\n");
        return -1;
    }

    // Leemos el contenido del archivo particion.bin, para después ir sacando la información de este mismo.
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
    memcpy(&ext_superblock, &datosfich[0], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, &datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, &datosfich[2], SIZE_BLOQUE);
    memcpy(&directorio, &datosfich[3], SIZE_BLOQUE);
    memcpy(&memdatos, &datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Buble for que actuará como si se tratase de un "while(1)", donde se llamará a los comando.
    for(;;){
        printf(">> ");
        fgets(comando, LONGITUD_COMANDO, stdin);
        comando[strcspn(comando, "\n")] = 0;

        // Verificamos que el usaurio haya introducido bien el comando.
        if (ComprobarComando(comando, orden, argumento1, argumento2) != 0) {
            printf("ERROR: Comando invalido. Intente nuevamente.\n"); // Salta´ra un error en caso de que el usuario introduzca un comando erroneo.
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
            }
        } 
        
        // ---REMOVE---
        else if (strcmp(orden, "remove") == 0) {
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == 0) {
                printf("Fichero eliminado con exito.\n");
            }
        } 

        // ---COPY---
        else if (strcmp(orden, "copy") == 0) {
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == 0) {
                printf("Fichero copiado con exito.\n");
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
            //cierra todo antes de salir del programa
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            GrabarByteMaps(&ext_bytemaps, fent);
            GrabarSuperBloque(&ext_superblock, fent);
            GrabarDatos(memdatos, fent);
            fclose(fent);
            printf("Saliendo del sistema de archivos....\n");
            break;
        } 

        // ---COMANDO DESCONOCIDO---
        else {
            printf("ERROR: Comando ilegal [%s].\n", orden);
        }
    }
    return 0;
}

// ---------------MÉTODOS---------------


// ---INFO--- / Método que mostrará la información del superbloque por la consola.
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {

    // Imprimimos la información.
    printf("Bloques %d bytes\n", psup->s_block_size);
    printf("Inodos particion: %d\n", psup->s_inodes_count);
    printf("Inodos libres: %d\n", psup->s_free_inodes_count);
    printf("Bloques particion: %d\n", psup->s_blocks_count);
    printf("Bloques libres: %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos: %d\n", psup->s_first_data_block);
}

// ---BYTEMAPS--- / Método que mostrará el contenido del "bytemap" de inodos y los 25 primeros elementos del bytemap de bloques.
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
        if (directorio[i].dir_inodo != 0xFFFF && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i; // Si la entrada es válida y el nombre coincide, devolvemos el valor.
        }
    }
    return -1; // Si no encuentra el directorio, o el nombre no coincide, el archivo no ha sido encontrado.
}

// ---DIR--- / Método que imprime el contenido del directorio, el tamaño, inodo, nombre y bloques que ocupa.
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    
    // Recorremos todas las entradas del directorio.
    for (int i = 0; i < MAX_FICHEROS; i++) {

        // Comprobamos si la entrada actual está asignada.
        if (directorio[i].dir_inodo != 0xFFFF) {

            // Guardamos en "InodoIdx" el índice del inodo asociado al archivo de la entrada del directorio.
            int inodoIdx = directorio[i].dir_inodo;

            // Imprimimos el nombre, tamaño, inido y bloques que ocupa.
            printf("%s tamanio:%d inodo:%d bloques:", directorio[i].dir_nfich, inodos->blq_inodos[inodoIdx].size_fichero, inodoIdx);
            
            // Recorremos los bloques asignados al archivo.
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {

                // Comprobamos si el valor del bloque es "0xFFFF", para saber si está asignado o no.
                if (inodos->blq_inodos[inodoIdx].i_nbloque[j] != 0xFFFF) {
                    printf(" %d", inodos->blq_inodos[inodoIdx].i_nbloque[j]);
                }
            }
            printf("\n"); // Salto de linea.
        }
    }
}

// ---RENAME--- / Método que cambiará el nombre del fichero en la entrada correspondiente.
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombredestino) {
    
    // Guardamos en "indice_antiguo" el nombre del archivo actual.
    int indice_antiguo = BuscaFich(directorio, inodos, nombreantiguo);
    
    // Guardamos en "indice_nuevo" el nombre nuevo del archivo.
    int indice_nuevo = BuscaFich(directorio, inodos, nombredestino);

    // Comprobamos si el archivo con el nombre antiguo existe.
    if (indice_antiguo == -1) {

        // Si el archivo no existe, se imprimirá el mensaje.
        printf("ERROR: Fichero %s no encontrado.\n", nombreantiguo);
        return -1;
    }

    // Comprobamos si ya existe un archivo con el nombre nuevo.
    if (indice_nuevo != -1) {
        printf("ERROR: Ya existe un fichero con el nombre %s.\n", nombredestino);
        return -1;
    }

    // Utilizamos la función de "strcpy" para copiar el nuevo nombre en el campo "dir.nfich" de la entrada del directorio correspondiente al archivo original.
    strcpy(directorio[indice_antiguo].dir_nfich, nombredestino);
    return 0;
}

// ---IMPRIMIR--- / Método que mostrará el contenido del fichero.
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    
    // Llamamos a la función de "BuscaFich" para encontrar el índice del archivo y lo guardamos en "indice".
    int indice = BuscaFich(directorio, inodos, nombre);

    // Verificamos si el archivo existe.
    if (indice == -1) {

        // Si el archivo no existe, imprimirá el mensaje.
        printf("ERROR: Fichero %s no encontrado.\n", nombre);
        return -1;
    }

    // Obtenemos un puntero al nodo que corresponde con el archivo.
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[indice].dir_inodo];
    
    // Recorremos los bloques de datos referenciados por el inodo del archivo.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {

        // Comprobamos que el valor no sea "0xFFFF", lo cual indicará que está vacio.
        if (inodo->i_nbloque[i] != 0xFFFF) {
            
            // Si está vacio, imprimirá el contenido almacenado en el bloque.
            printf("%s", (char *)memdatos[inodo->i_nbloque[i]].dato);
        }
    }
    printf("\n"); // Salto de linea.
    return 0;
}

// ---REMOVE--- / Método que borrará el fichero que le indiquemos.
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    
    // Llamamos a la función de "BuscaFich" para encontrar el índice del archivo y lo guardamos en "indice".
    int indice = BuscaFich(directorio, inodos, nombre);
   
    // Si el indice guardado anteriormente es "-1", imprimimos un mensaje de "ERROR" indicando que el archivo no existe.
    if (indice == -1) {
        printf("ERROR: Fichero %s no encontrado.\n", nombre);
        return -1;
    }
    
    // Obtenemos un puntero al nodo que corresponde con el archivo.
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[indice].dir_inodo];
    
    // Recorremos los bloques asignados al inodo.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        
        // Comprobamos que el valor no sea "0xFFFF", lo cual indicará que está vacio.
        if (inodo->i_nbloque[i] != 0xFFFF) {
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            inodo->i_nbloque[i] = 0xFFFF;
        }
    }

    // Marcamos al inodo como libre en el mapa de bits, y dejamos su valor en 0.
    ext_bytemaps->bmap_inodos[directorio[indice].dir_inodo] = 0;
    inodo->size_fichero = 0; // Ponemos su valor a 0.

    // Marcamos la entrada del directorio como "no válida".
    directorio[indice].dir_inodo = 0xFFFF;

    // Limpiamos el nombre del archivo.
    strcpy(directorio[indice].dir_nfich, "");

    ext_superblock->s_free_inodes_count++; // Inodos libres en el superbloque.
    ext_superblock->s_free_blocks_count++; // Bloques libres en el superbloque.
    return 0;
}

// ---COPY--- / Método que copiará el fichero en uno nuevo.
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
   
    // Llamamos a la función de "BuscaFich" para encontrar el índice del archivo y lo guardamos en "indice".
    int indice_origen = BuscaFich(directorio, inodos, nombreorigen);
   
    // Si el indice guardado anteriormente es "-1", imprimimos un mensaje de "ERROR" indicando que el archivo no existe.
    if (indice_origen == -1) {
        printf("ERROR: Fichero origen %s no encontrado.\n", nombreorigen);
        return -1;
    }

    // Comprobamos si ya existe un archivo con el nombre nuevo.
    int indice_destino = BuscaFich(directorio, inodos, nombredestino);
    if (indice_destino != -1) {
        printf("ERROR: Ya existe un fichero con el nombre %s.\n", nombredestino);
        return -1;
    }

    int inodo_libre = -1;

    // Recorremos todo el mapa de bits para buscar un inodo que esté libre.
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            inodo_libre = i;
            break;
        }
    }

    if (inodo_libre == -1) {
        printf("ERROR: No hay inodos libres.\n");
        return -1;
    }
    
    // Buscamos una posición libre en el directorio donde "dir_inodo" sea 0xFFFF.
    int directorio_libre = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == 0xFFFF) {
            directorio_libre = i;
            break;
        }
    }

    // Comprobamos si hay entredas libres en el directorio.
    if (directorio_libre == -1) {

        // Si no hay mas entradas, saltará un ERROR.
        printf("ERROR: No hay entradas libres en el directorio.\n");
        return -1;
    }

    // Obtenemos los punteros al inodo del archivo de origen y el nuevo inodo de destino.
    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[indice_origen].dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[inodo_libre];
    inodo_destino->size_fichero = inodo_origen->size_fichero;

    // Recorremos los bloques asignados al archivo de origen para buscar un bloque libre en el mapa de bloques.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo_origen->i_nbloque[i] != 0xFFFF) {
            int bloque_libre = -1;
            for (int j = 0; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloque_libre = j;
                    break;
                }
            }

            // En caso de que no haya bloques libres, saltará un error.
            if (bloque_libre == -1) {
                printf("ERROR: No hay bloques libres para copiar el archivo.\n");
                return -1;
            }

            // Marcamos el bloque como "ocupado" en el mapa de bits con " = 1". 
            ext_bytemaps->bmap_bloques[bloque_libre] = 1;

            // Asociamos el bloque al inodo de destino.
            inodo_destino->i_nbloque[i] = bloque_libre;

            // Copiamos con "memcpy" el contenido del bloque de origen al bloque de destino.
            memcpy(&memdatos[bloque_libre], &memdatos[inodo_origen->i_nbloque[i]], SIZE_BLOQUE);
            }
            else {
            inodo_destino->i_nbloque[i] = 0xFFFF;
        }
    }

    // Actualizamos el nuevo inodo y lo marcamos como "ocupado".
    ext_bytemaps->bmap_inodos[inodo_libre] = 1;

    // Añadimos el nuevo archivo al directorio con el nombre e inodo asociados.
    strcpy(directorio[directorio_libre].dir_nfich, nombredestino);
    directorio[directorio_libre].dir_inodo = inodo_libre;
    return 0;
}

// ---------------GUARDADO DE DATOS---------------

// ---GRABAR INODOS Y DIRECTORIO---- / Método para guardar los inodos y el directorio.
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo hasta la posición que le corresponda.
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);

    // Escribimos con "fwrite" una copia de la estructura de inodos en el archivo, y le indicamos que ocupe solo un bloque.
    fwrite(inodos, SIZE_BLOQUE, 1, fich);

    // Escribimos la estructura del directorio en el archivo inmediatamente después de los inodos, y le indicamos que ocupe 1 bloque.
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR BYREMAPS---- / Método para guardar los mapas de bits.

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo al segundo bloque del archivo, que es donde se almacenan los mapas de bits.
    fseek(fich, SIZE_BLOQUE, SEEK_SET);

    // Con "fwrite" escribimos los mapas de bits en ese mismo bloque.   
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR SUPERBLOQUE---- / Método para guardar el superbloque.
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    
    // Usamos "fseek" para desplazar el puntero del archivo al principio del archivo o primer bloque.
    fseek(fich, 0, SEEK_SET);
    
    // Con "fwite" escribimos la estructura del superbloque en el primer bloque.
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

// ---GRABAR DATOS---- / Método para guardar los datos.
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    // Usamos "fseek" para desplazar el puntero del archivo hasta el cuato bloque del archivo, que es donde inicia el área de datos.
    fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);

    // Escribimos con "fwrite" todos los bloques de datos desde el cuarto bloque (donde inicia el área de datos) en adelante
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

