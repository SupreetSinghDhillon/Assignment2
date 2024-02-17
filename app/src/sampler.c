#include "sampler.h"
#include "hal/lightSensor.h"

#define SAMPLING_INTERVAL_MS 1

#define ALPHA 0.999
#define ONE_MINUS_ALPHA (1 - ALPHA)

#define MAX_HIST 1000
#define TEMP_BUFF_SIZE 1000

static double currentAvg = 0.0;

static pthread_t samplerThread;
static pthread_mutex_t historyLock = PTHREAD_MUTEX_INITIALIZER;
static bool samplingThreadRunning = false;

static long long numSamplesTaken = 0;
static double history[MAX_HIST];
static int historySize = 0;

static double tempBuffer[TEMP_BUFF_SIZE];
static int tempBufferSize = 0;


void calculateExpMovingAvg(double newSample) 
{
    pthread_mutex_lock(&historyLock);
    currentAvg = (ALPHA * currentAvg) + (ONE_MINUS_ALPHA * newSample);
    pthread_mutex_unlock(&historyLock);
}

void* samplingFunction(void* arg)
{
    (void)arg;
    while (samplingThreadRunning) {
        double sample = getLightSensorReading();
        if(tempBufferSize < TEMP_BUFF_SIZE) {
            tempBuffer[tempBufferSize++] = sample;
        }
        calculateExpMovingAvg(sample);
        numSamplesTaken++;

        usleep(SAMPLING_INTERVAL_MS * 1000);
    }
    return NULL;
}

void Sampler_init(void)
{
    if (!samplingThreadRunning) {
        samplingThreadRunning = true;
        pthread_create(&samplerThread, NULL, samplingFunction, NULL);
    }
}

void Sampler_cleanup(void)
{
    if(samplingThreadRunning) {
        samplingThreadRunning = false;
        pthread_join(samplerThread, NULL);
        pthread_mutex_destroy(&historyLock);
    }
}

void Sampler_moveCurrentDataToHistory(void)
{
    pthread_mutex_lock(&historyLock);
    if(tempBufferSize > 0) {
        int remainingSpace = sizeof(history) / sizeof(history[0]) - historySize;
        int numToMove = (tempBufferSize <= remainingSpace) ? tempBufferSize : remainingSpace;

        for (int i = 0; i < numToMove; i++) {
            history[historySize++] = tempBuffer[i];
        }

        for (int i = numToMove; i < tempBufferSize; i++) {
            tempBuffer[i - numToMove] = tempBuffer[i];
        }

        tempBufferSize -= numToMove;
    }
    pthread_mutex_unlock(&historyLock);
}

int Sampler_getHistorySize(void)
{
    return historySize;
}

double Sampler_getAverageReading(void)
{
    return currentAvg;
}

long long Sampler_getNumSamplesTaken(void) 
{
    return numSamplesTaken;
}

