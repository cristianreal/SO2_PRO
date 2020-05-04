/**
 * Modulo de registro de cambios (INOTIFY)
 * cambios: borrado, modificacion y creacion de archivos
 * monitoreo de la raiz
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <assert.h>
#include <time.h>

#define EVENT_SIZE (sizeof(struct inotify_event) + 24)
#define BUF_LEN (1024 * EVENT_SIZE)

void writeinFile(char* datetime, char* desc, char* infile){
	FILE *fptr;
	fptr = fopen("/home/ing-usac-201513700/log_proyecto","a");//mode a: create or write at EOF
	if(fptr == NULL){
		printf("Error!");
		exit(1);
	}
	fprintf(fptr,"time: %s <-> desc: %s <-> name: %s.\n", datetime, desc, infile);
	fclose(fptr);
}

//struct inotify_event {
//   int      wd;       /* Watch descriptor */
//   uint32_t mask;     /* Mask describing event */
//   					/* contains bits that describe the event that occurred */
//   uint32_t cookie;   /* Unique cookie associating related
//                         events (for rename(2)) */
//   uint32_t len;      /* Size of name field */
//   char     name[];   /* Optional null-terminated name */
//};

int main(int argc, char **argv){
	char buffer[BUF_LEN];
	int file_descriptor = inotify_init();
	
	if (file_descriptor < 0){
		perror("[SO2-ing-usac-201513700] Error de inicio inotify.");
	}

	//indicate the directory to monitor
	//IN_MODIFY: File was modified
	//IN_CREATE: File/directory created in watched directory
	//IN_DELETE: File/directory deleted from watched directory.
	int watch_descriptor = inotify_add_watch(file_descriptor, "/", IN_MODIFY | IN_CREATE | IN_DELETE);
	
	//options during monitoring
	if (watch_descriptor == -1){
		printf("[SO2-ing-usac-201513700] Error al monitorear el directorio %s\n", "/");
	}else{
		printf("[SO2-ing-usac-201513700] Monitoreando el directorio %s con inotify\n", "/");
	}

	//Actions in root directory
	while(1){

		//----------datetime--------
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		char s[64];
		assert(strftime(s, sizeof(s), "%c", tm));
		//--------------------------
		
		printf("\033[0m"); //Resets the text to default color
		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0){
			perror("read");
		}
		
		int offset = 0;
		while (offset < length){
			struct inotify_event *event = (struct inotify_event *)&buffer[offset];

			if (event->len){
				
				if (event->mask & IN_CREATE){
					
					printf("\033[0;32m"); //Set the text to the color green
					if (event->mask & IN_ISDIR){
						printf("%s | directorio %s creado.\n", s, event->name);
						writeinFile(s, "directorio creado", event->name);
					}else{
						printf("%s | archivo %s creado.\n", s, event->name);
						writeinFile(s, "archivo creado", event->name);
					}

				}else if (event->mask & IN_DELETE){

					printf("\033[0;31m"); //Set the text to the color red
					if (event->mask & IN_ISDIR){
						printf("%s | directorio %s eliminado.\n", s, event->name);
						writeinFile(s, "directorio eliminado", event->name);
					}else{
						printf("%s El archivo %s fue eliminado.\n", s, event->name);
						writeinFile(s, "archivo eliminado", event->name);
					}

				}else if (event->mask & IN_MODIFY){

					printf("\033[0;33m"); //Set the text to the color yellow
					if (event->mask & IN_ISDIR){
						printf("%s | directorio %s modificado.\n", s, event->name);
						writeinFile(s, "directorio modificado", event->name);
					}else{
						printf("%s | archivo %s modificado.\n", s, event->name);
						writeinFile(s, "archivo modificado", event->name);
					}

				}
			}

			offset += sizeof(struct inotify_event) + event->len;

		}

	}

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	return EXIT_SUCCESS;
}