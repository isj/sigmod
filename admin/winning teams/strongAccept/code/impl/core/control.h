#ifndef CONTROL_H
#define CONTROL_H

const int MAX_TASK = 5;

extern __thread int workerID;

typedef void (*Starter)();
typedef void (*Handler)(int);

void registerStarter(Starter function);
int registerTask(Handler function);

void startThreads();
void stopThreads();

void doTask(int taskID, int capacity);

#endif

