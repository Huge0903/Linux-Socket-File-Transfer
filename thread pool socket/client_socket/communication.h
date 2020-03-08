#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "public.h"

void OrderCompiler(int client_fd, const char * order);
void fileReceive(int client_fd, const char * filename);
ssize_t readLine(int fd, char * buffer, size_t n);

#endif