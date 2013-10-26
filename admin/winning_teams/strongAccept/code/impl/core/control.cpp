#include "core.h"

using namespace std;

__thread int workerID;

static Starter starter;
static Handler handlers[MAX_TASK];

static atomic<bool> endFlag;
static atomic<bool> workFlags[MAX_TASK];
static atomic<int> startCounters[MAX_TASK];
static atomic<int> finishCounters[MAX_TASK];
static thread workers[THREADS];
static int capacities[MAX_TASK];
static int taskCounter = 0;

static void doTaskInternal(int taskID);

void registerStarter(Starter function)
{
	starter = function;
}

int registerTask(Handler function)
{
	handlers[taskCounter] = function;
	return taskCounter++;
}

void startThreads()
{
	for (int k = 1; k < THREADS; k++)
	{
		workers[k] = thread([k](){
			workerID = k;
			starter();
			while (true)
			{
				if (endFlag) break;
				for (int t = 0; t < taskCounter; t++)
				{
					if (workFlags[t])
						doTaskInternal(t);
				}
			}
		});
	}
	workerID = 0;
	starter();
}

void stopThreads()
{
	endFlag = true;
	for (int k = 1; k < THREADS; k++)
		workers[k].join();
}

void doTask(int taskID, int capacity)
{
	startCounters[taskID] = 0;
	finishCounters[taskID] = 0;
	capacities[taskID] = capacity;
	workFlags[taskID] = true;

	doTaskInternal(taskID);
	while (finishCounters[taskID] < capacities[taskID]);

	workFlags[taskID] = false;
}

void doTaskInternal(int taskID)
{
	int id;
	while ((id = startCounters[taskID]++) < capacities[taskID])
	{
		handlers[taskID](id);
		finishCounters[taskID]++;
	}
}

