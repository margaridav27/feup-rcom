#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

int init(char* filename, char* port);

void setID(int id);

int sendCtrlPacket(unsigned char ctrl);

int sendDataPacket(unsigned char* data, unsigned char seq_num, int read_sz);

int communicate(char* filename, char* port);

void end(unsigned char* filename, unsigned char* port);

#endif