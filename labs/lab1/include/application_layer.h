#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

int init(char* filename, char* port);

int createCtrlPacket(char ctrl, char* packet);

int createDataPacket();

int emmit(char* filename, char* port);

void end(char* filename, char* port);

#endif