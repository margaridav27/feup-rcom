#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

int init(char* filename, char* port);

void setID(int id);

int createCtrlPacket(unsigned char ctrl, unsigned char* packet);

int createDataPacket(unsigned char* data,
                     unsigned char seq_num,
                     unsigned char* packet);

int communicate(char* filename, char* port);

void end(unsigned char* filename, unsigned char* port);

#endif