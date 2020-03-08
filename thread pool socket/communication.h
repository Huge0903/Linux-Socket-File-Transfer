#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "public.h"

void OrderCompiler(int client_fd, const char * order);
void fileTransfer(int client_fd, const char * filename);

#endif