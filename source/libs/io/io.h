#ifndef IO_H
#define IO_H

void* io_image_load(char* path, int* w, int *h, int* chanels);
void  io_image_unload(void* data);


#endif