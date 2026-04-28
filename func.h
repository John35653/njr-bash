#include <unistd.h> // so that pid_t doesn't throw an error

void cwdHandler();
void handleSigint(int sig);
void pid_check(pid_t pid);