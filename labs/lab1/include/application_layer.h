#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

int llopen(char* filename, char* port);

int createPacket();

int emmit(char* filename, char* port);

void llclose(char* filename, char* port);

#endif