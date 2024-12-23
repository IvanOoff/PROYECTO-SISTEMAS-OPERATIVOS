#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#define LONGITUD_COMANDO 100

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

// Main del programa.
int main(){

	 char comando[LONGITUD_COMANDO];
	 char orden[LONGITUD_COMANDO];
	 char argumento1[LONGITUD_COMANDO];
	 char argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos = 0;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     fent = fopen("particion.bin", "r+b");
     if (fent == NULL) {
        printf("ERROR\n");
        return -1;
        }

     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
 for (;;) {
        do {
            printf(">> ");
            fflush(stdin);
            fgets(comando, LONGITUD_COMANDO, stdin);
        } while (ComprobarComando(comando, orden, argumento1, argumento2) != 0);

        // ---INFO---
        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
            continue;
        }

        //---BYTEMAPS---
        if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
            continue;
        }

        // ---DIR---
        if (strcmp(orden, "dir") == 0) {
            Directorio(&directorio, &ext_blq_inodos);
            continue;
        }

        // ---RENAME---
        if (strcmp(orden, "rename") == 0) {
            Renombrar(&directorio, &ext_blq_inodos, argumento1, argumento2);
        } 

        // ---REMOVE---
        else if (strcmp(orden, "remove") == 0) {
            Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
        } 

        // ---COPY---
        else if (strcmp(orden, "copy") == 0) {
            Copiar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, &memdatos, argumento1, argumento2, fent);
        } 

        // ---IMPRIMIR---
        else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(&directorio, &ext_blq_inodos, &memdatos, argumento1);
        } 

        // ---SALIR---
        else if (strcmp(orden, "salir") == 0) {
            GrabarDatos(&memdatos, fent);
            fclose(fent);
            return 0;
        } 

        // En el caso de que el usuario introduzca un comando que no exista o se equivoque, saltará un mensaje.
        else {
            printf("COMANDO NO ENCONTRADO: %s\n", orden);
        }

         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
     }
}

   // ----INFO----

   //Método que mostrará la información por pantalla del "superbloque".
   void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
      printf("Bloque %d bytes\n" , SIZE_BLOQUE); //"SIZE_BLOQUE" coincide con los 512 bytes de tamaño del bloque.
      printf("Inodos particion = %d\n" , psup->s_inodes_count);
      printf("Inodos libres = %d\n" , psup->s_free_inodes_count);
      printf("Bloques particion = %d\n" , psup->s_blocks_count);
      printf("Bloques libres = %d\n" , psup->s_free_blocks_count);
      printf("Primer bloque de datos = %d\n" , psup->s_first_data_block);
   }
 
   //----BYTEMAPS----

   //Método que mostrará el contenido del bytemap de inodos y los 25 primeros elementos del bytemap de bloques.
   void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {

      //INODOS.
      printf("Inodos :");

      //Hacemos un for para imprimir el estado o bytemap de los inodos -> 0 = No ocupado, 1 = Ocupado.
      for (int i = 0; i < MAX_INODOS; i++) {
         printf("%d ", ext_bytemaps->bmap_inodos[i]);
         }
         printf("\n"); //Saltamos de linea.
         
         //BLOQUES.
         printf("Bloques [0-25] :");
         
         // Hacemos un for para imprimir el estado o bytemap de los primeros 25 bloques.
         for (int j = 0; j < 25; j++) {
            printf("%d ", ext_bytemaps->bmap_bloques[j]);
            }
            
            printf("\n"); //Saltamos de linea.
   }

   //----DIR----

   //Método que mostrará los ficheros, tanto su nombre, tamaño y bloques.
   void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
      
       //Hacemos un bucle for y empezamos en "1" para evitar la entrada especial la cual es la carpeta "raiz", y calculamosmel maximo de entradas de archivo que puede tener el directorio.
       for (int k= 1; k < MAX_FICHEROS; k++) {
         
         //Obtenemos el índice del inodo de la entrada del directorio en la posición "k".
         int index_i = directorio[k].dir_inodo;
         
         //Verificamos qe el inodo sea válido (0xFFFF).
         if (index_i != 0xFFFF) {
            
            //Imprimimos tanto el nombre, tamaño e inodo.
            printf("%s tamanio:%d inodo:%d bloques: ", directorio[k].dir_nfich, inodos->blq_inodos[index_i].size_fichero, index_i);
            int p_bloque = 1;
            
            //Hacemos un bucle para recorrer los bloques del archivo.
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos->blq_inodos[index_i].i_nbloque[j] != 0xFFFF) {  //Si está ocupado, sale.
                    if (!p_bloque) {
                        printf(" "); //Espacio entre bloques.
                    }
                    printf("%d", inodos->blq_inodos[index_i].i_nbloque[j]);
                    p_bloque = 0;  //Después del primer bloque, agrega un espacio entre los bloques.
                }
            }
            printf("\n");  //Saltamos de linea.
        }
    }
}
/*
   //----RENAME----

   //Método utilizado para renombrar 
   int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    
    //----VARIABLES----
    int old_exists = -1; //Archivo -> Nombre antiguo.
    int new_exists = -1; //Archivo -> Nombre nuevo.
    
    //Buscamos el fichero en el directorio para si existe un archivo con el nombre antiguo y con el nombre nuevo.
    for (int i = 0; i < MAX_FICHEROS; i++) {

        //NOMBRE ANTIGUO.
        if (directorio[i].dir_inodo != 0xFFFF) {
            if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) {
                old_exists = i;
                }

                //NOMBRE NUEVO.
                if (strcmp(directorio[i].dir_nfich, nombrenuevo) == 0) {
                    new_exists = i;
                    }
                    }
                    }

    //Si no se encuentra el fichero con el nombre antiguo imprimiremos el mensaje.
    if (old_exists == -1) {
        printf("Fichero %s NO existe.\n", nombreantiguo);
        return -1;
    }

    //Verificamos si ya existe un fichero con el nombre nuevo.
    if (new_exists != -1) {
        printf("Fichero %s ya existe.\n", nombrenuevo);
        return -1;
    }

    //Usamos el "strcpy" para cambiar el nombre antiguo por le nuevo.
    strcpy(directorio[old_exists].dir_nfich, nombrenuevo);
    return 0;
   }

   //----IMPRIMIR----
   int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    
    //Introducimos en la variable "inodoInx" la información del nombre, inodos y directorio.
    int inodoInx = BuscaFich(directorio, inodos, nombre);

    //Obtenemos el inodo del archivo.
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodoInx];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != 0xFFFF) {
            printf("%s", (char *)memdatos[inodo->i_nbloque[i]]);
        }
    }
    printf("\n"); // Salto de linea.
    return 0;
   }
   

   //----REMOVE / BORRAR----

   // Método para borrar lo
   int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    
    // Buscamos el archivo para después poder borrarlo "BuscaFich".
    int i_index = BuscaFich(directorio, inodos, nombre);

    // Verificamos si el archi existe, sino lanzará el mensaje de ERROR. 
    if (i_index == -1) {
        printf("ERROR: Fichero %s no encontrado\n", nombre);
        return -1;
        }
    
    // Liberamos los bloques de datos del inodo.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodos->blq_inodos[i_index].i_nbloque[i] != 0xFFFF) {
            ext_bytemaps->bmap_bloques[inodos->blq_inodos[i_index].i_nbloque[i]] = 0;
            inodos->blq_inodos[i_index].i_nbloque[i] = 0xFFFF;
        }
    }

    // Marcamos el inodo como libre "0".
    ext_bytemaps->bmap_inodos[i_index] = 0;

    // Reiniciamos el tamaño de archivo
    inodos->blq_inodos[i_index].size_fichero = 0;

    // Eliminamos la posibilidad de entrada del archivo al directorio.
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == i_index) {
            strcpy(directorio[i].dir_nfich, "");
            directorio[i].dir_inodo = 0xFFFF;
            break;
        }
    }
    
    // Incrementamos el contador de inodos libres en el super-bloque.
    ext_superblock->s_inodos_libres++;
    return 0;
   }

   //----SALIR----

   // Método para salir.
   if (strcmp(orden, "salir") == 0) {

    // 1. Guardamos la estructura del directorio y la lista de los inodos en el archivo "FENT".
    Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent); // Así nos aseguramos que ningun archivo o tamaño se pierda.

    // 2. Guardamos los mapas de bits de bloques e inodos.
    GrabarByteMaps(&ext_bytemaps, fent);

    // 3. Guardamos el superbloque que contiene información global sobre el sistema de archivos.
    GrabarSuperBloque(&ext_superblock, fent);

    // 4. Guarmos el contenido de los bloques de datos en el archivo del sistema.
    GrabarDatos(&memdatos, fent);

    // 5. Por último cerramos el archivo.
    fclose(fent);

    // Imprimimos el mensaje para informar al usuario de que ha salido.
    printf("CERRADO DE ARCHIVOS COMPLETADO CON EXITO\n");
    return 0;
   }
   

   //----COPIAR----

   //Método para copiar un archivo dentro del sistema de archivos simulado.
   int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {

  // Buscamos el archivo de origen "nombreOrigen" en el directorio.
    int i_origen = BuscaFich(directorio, inodos, nombreorigen);

   // Buscar un inodo libre.
   int i_libre = -1;

   // Recorremos el "mapa" de bits de inodos para encontrar un inodo que esté disponible.
   for (int i = 0; i < MAX_INODOS; i++) {
    if (ext_bytemaps->bmap_inodos[i] == 0) {
        i_libre = i; // El primer índice libre se guardará en la variable "i_libre".
        break;
        }
        }

    // Buscamos un espacio libre en el directorio para registrar el nuevo archivo.
    int entrada_libre = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == 0xFFFF) {
            entrada_libre = i; // Lo guardamos.
            break;
        }
    }

    // Copiamos los datos del fichero origen al fichero destino.
    EXT_SIMPLE_INODE *inode_origen = &inodos->blq_inodos[i_origen];
    EXT_SIMPLE_INODE *inode_destino = &inodos->blq_inodos[i_libre];

    // Copiamos el tamaño del archivo.
    inode_destino->size_fichero = inode_origen->size_fichero;

    // Hacemos un bucle para recorrer los bloques del archivo origen y lo copiamos al destino.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode_origen->i_nbloque[i] != 0xFFFF) {
            int bloque_libre = -1;
            for (int j = 0; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloque_libre = j;
                    break;
                }
            }

            // Actualizamos los mapas de bits.
            ext_bytemaps->bmap_bloques[bloque_libre] = 1;
            inode_destino->i_nbloque[i] = bloque_libre;
            memcpy(&memdatos[bloque_libre], &memdatos[inode_origen->i_nbloque[i]], SIZE_BLOQUE);
        } else {
            inode_destino->i_nbloque[i] = 0xFFFF;
        }
   }

    // Actualizamos los mapas de bits.
    ext_bytemaps->bmap_inodos[i_libre] = 1;

    // Registramos el nuevo archivo en el directorio.
    strcpy(directorio[entrada_libre].dir_nfich, nombredestino);
    directorio[entrada_libre].dir_inodo = i_libre;

    return 0; // Vuelve.
}


   //----COMANDO DESCONOCIDO----

// Método que lanzará un error si el usuario pide un comando
if (strcmp(orden, "listar") == 0) {
    ListarArchivos(&directorio, &ext_blq_inodos);
} else if (strcmp(orden, "borrar") == 0) {
    Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, &memdatos, nombreorigen, nombredestino, fent);
} else if (strcmp(orden, "listar") == 0) {
    Listar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, nombre, fent);
} else if (strcmp(orden, "copiar") == 0) {
    Copiar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, &memdatos, fent);
} else {
    printf("ERROR: Comando ilegal [%s]", orden);
}*/