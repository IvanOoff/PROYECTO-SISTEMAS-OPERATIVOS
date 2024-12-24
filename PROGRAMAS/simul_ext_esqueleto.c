#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#define LONGITUD_COMANDO 100

// Declaración de las funciones utilizadas en el programa
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

//Main del programa

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

    //apertura del archivo

    FILE *fent = fopen("particion.bin", "r+b");
    if (!fent) {
        printf("ERROR: No se pudo abrir el archivo particion.bin\n");
        return -1;
    }

    // Leer contenido del archivo particion.bin
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
    memcpy(&ext_superblock, &datosfich[0], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, &datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, &datosfich[2], SIZE_BLOQUE);
    memcpy(&directorio, &datosfich[3], SIZE_BLOQUE);
    memcpy(&memdatos, &datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    //bucle comandos
    for(;;){
        printf(">> ");
        fgets(comando, LONGITUD_COMANDO, stdin);
        comando[strcspn(comando, "\n")] = 0;
        //verifica el comando
        if (ComprobarComando(comando, orden, argumento1, argumento2) != 0) {
            printf("ERROR: Comando inválido. Intente nuevamente.\n");
            continue;
        }
        //INFO
        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        } 
        //BYTEMAPS
        else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } 
        //DIR
        else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } 
        //RENAME
        else if (strcmp(orden, "rename") == 0) {
            if (Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == 0) {
                printf("Fichero renombrado con éxito.\n");
            }
        } 
        //REMOVE
        else if (strcmp(orden, "remove") == 0) {
            if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == 0) {
                printf("Fichero eliminado con éxito.\n");
            }
        } 
        //COPY
        else if (strcmp(orden, "copy") == 0) {
            if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == 0) {
                printf("Fichero copiado con éxito.\n");
            }
        } 
        //IMPRIMIR
        else if (strcmp(orden, "imprimir") == 0) {
            if (Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1) == 0) {
                printf("Contenido impreso con éxito.\n");
            }
        } 
        //SALIR
        else if (strcmp(orden, "salir") == 0) {
            //cierra todo antes de salir del programa
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            GrabarByteMaps(&ext_bytemaps, fent);
            GrabarSuperBloque(&ext_superblock, fent);
            GrabarDatos(memdatos, fent);
            fclose(fent);
            printf("Saliendo del sistema de archivos.\n");
            break;
        } 
        //COMANDO DESCONOCIDO
        else {
            printf("ERROR: Comando desconocido [%s].\n", orden);
        }
    }

    return 0;
}


// Implementación de las funciones


//IMPRIME BYTEMAPS
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\nBloques [0-25]: ");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}
//verifica el comando
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    return sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2) >= 1 ? 0 : -1;
}
//printea el superbloque
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Bloques: %d bytes\n", psup->s_block_size);
    printf("Inodos particion: %d\n", psup->s_inodes_count);
    printf("Inodos libres: %d\n", psup->s_free_inodes_count);
    printf("Bloques particion: %d\n", psup->s_blocks_count);
    printf("Bloques libres: %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos: %d\n", psup->s_first_data_block);
}
//busca el fichero que se pregunta
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != 0xFFFF && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i;
        }
    }
    return -1;
}
//imprime el contenido del directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != 0xFFFF) {
            int inodoIdx = directorio[i].dir_inodo;
            printf("%s tamanio:%d inodo:%d bloques:", directorio[i].dir_nfich, inodos->blq_inodos[inodoIdx].size_fichero, inodoIdx);
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos->blq_inodos[inodoIdx].i_nbloque[j] != 0xFFFF) {
                    printf(" %d", inodos->blq_inodos[inodoIdx].i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}
//cambia el nombre del fichero
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombredestino) {
    int indice_antiguo = BuscaFich(directorio, inodos, nombreantiguo);
    int indice_nuevo = BuscaFich(directorio, inodos, nombredestino);

    if (indice_antiguo == -1) {
        printf("ERROR: Fichero %s no encontrado.\n", nombreantiguo);
        return -1;
    }
    if (indice_nuevo != -1) {
        printf("ERROR: Ya existe un fichero con el nombre %s.\n", nombredestino);
        return -1;
    }

    strcpy(directorio[indice_antiguo].dir_nfich, nombredestino);
    return 0;
}
//muestra el contenido del fichero
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int indice = BuscaFich(directorio, inodos, nombre);
    if (indice == -1) {
        printf("ERROR: Fichero %s no encontrado.\n", nombre);
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[indice].dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != 0xFFFF) {
            printf("%s", (char *)memdatos[inodo->i_nbloque[i]].dato);
        }
    }
    printf("\n");
    return 0;
}
//borra el fichero
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    int indice = BuscaFich(directorio, inodos, nombre);
    if (indice == -1) {
        printf("ERROR: Fichero %s no encontrado.\n", nombre);
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[indice].dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != 0xFFFF) {
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            inodo->i_nbloque[i] = 0xFFFF;
        }
    }

    ext_bytemaps->bmap_inodos[directorio[indice].dir_inodo] = 0;
    inodo->size_fichero = 0;
    directorio[indice].dir_inodo = 0xFFFF;
    strcpy(directorio[indice].dir_nfich, "");

    ext_superblock->s_free_inodes_count++;
    ext_superblock->s_free_blocks_count++;
    return 0;
}
//copia el fichero en uno nuevo
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    int indice_origen = BuscaFich(directorio, inodos, nombreorigen);
    if (indice_origen == -1) {
        printf("ERROR: Fichero origen %s no encontrado.\n", nombreorigen);
        return -1;
    }

    int indice_destino = BuscaFich(directorio, inodos, nombredestino);
    if (indice_destino != -1) {
        printf("ERROR: Ya existe un fichero con el nombre %s.\n", nombredestino);
        return -1;
    }

    int inodo_libre = -1;
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

    int directorio_libre = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == 0xFFFF) {
            directorio_libre = i;
            break;
        }
    }

    if (directorio_libre == -1) {
        printf("ERROR: No hay entradas libres en el directorio.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[indice_origen].dir_inodo];
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[inodo_libre];
    inodo_destino->size_fichero = inodo_origen->size_fichero;

    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo_origen->i_nbloque[i] != 0xFFFF) {
            int bloque_libre = -1;
            for (int j = 0; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloque_libre = j;
                    break;
                }
            }

            if (bloque_libre == -1) {
                printf("ERROR: No hay bloques libres para copiar el archivo.\n");
                return -1;
            }

            ext_bytemaps->bmap_bloques[bloque_libre] = 1;
            inodo_destino->i_nbloque[i] = bloque_libre;
            memcpy(&memdatos[bloque_libre], &memdatos[inodo_origen->i_nbloque[i]], SIZE_BLOQUE);
        } else {
            inodo_destino->i_nbloque[i] = 0xFFFF;
        }
    }

    ext_bytemaps->bmap_inodos[inodo_libre] = 1;
    strcpy(directorio[directorio_libre].dir_nfich, nombredestino);
    directorio[directorio_libre].dir_inodo = inodo_libre;
    return 0;
}

//guardado de datos

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

