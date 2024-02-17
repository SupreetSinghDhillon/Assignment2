#include "sampler.h"
#include "hal/lightSensor.h"

#define SAMPLING_INTERVAL_MS 1

#define ALPHA 0.999
#define ONE_MINUS_ALPHA (1 - ALPHA)

#define MAX_SIZE 1000

typedef struct {
    double* currBuffer;
    double* historyBuffer;
    int currBufferSize;
    int historyBufferSize;
    double currentAvg;
    long long numSamplesTaken;
    pthread_mutex_t lock;
    struct timespec startTime;
} SampleData;

static SampleData sampleData;
static pthread_t samplerThread;
static volatile bool samplingThreadRunning = false;

void calculateExpMovingAvg(double newSample) 
{
    pthread_mutex_lock(&sampleData.lock);
    sampleData.currentAvg = (ALPHA * sampleData.currentAvg) + (ONE_MINUS_ALPHA * newSample);
    pthread_mutex_unlock(&sampleData.lock);
}

void* samplingFunction(void* arg)
{
    printf("entering thread function\n");
    (void)arg;
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    sampleData.startTime = startTime;

    while (samplingThreadRunning) {
        struct timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        long elapseMs = (currentTime.tv_sec - sampleData.startTime.tv_sec) * 1000 + (currentTime.tv_nsec - sampleData.startTime.tv_nsec) / 1000000;
        
        if (elapseMs >= 1000) {
            Sampler_moveCurrentDataToHistory();
            sampleData.startTime = currentTime;
        }

        double sample = getLightSensorReading();
        printf("%f\n", sample);
        //pthread_mutex_lock(&sampleData.lock);
        if(sampleData.currBufferSize < MAX_SIZE) {
            sampleData.currBuffer[sampleData.currBufferSize++] = sample;
            calculateExpMovingAvg(sample);
            sampleData.numSamplesTaken++;
        }
       // pthread_mutex_unlock(&sampleData.lock);
        
        usleep(SAMPLING_INTERVAL_MS * 1000);
    }
    return NULL;
}

void Sampler_init(void)
{
    memset(&sampleData, 0, sizeof(SampleData));
    pthread_mutex_init(&sampleData.lock, NULL);
    sampleData.currBuffer = (double*)malloc(MAX_SIZE * sizeof(double));
    sampleData.historyBuffer = (double*)malloc(MAX_SIZE * sizeof(double));
    sampleData.currBufferSize = 0;
    sampleData.historyBufferSize = 0;
    sampleData.numSamplesTaken = 0;
    samplingThreadRunning = true;
    sampleData.currentAvg = 0.0;
    pthread_create(&samplerThread, NULL, samplingFunction, NULL);
    printf("initiating thread\n");
    
}

void Sampler_cleanup(void)
{
    samplingThreadRunning = false;
    pthread_join(samplerThread, NULL);
    pthread_mutex_destroy(&sampleData.lock);
    free(sampleData.currBuffer);
    free(sampleData.historyBuffer);
}

void Sampler_moveCurrentDataToHistory(void)
{
    pthread_mutex_lock(&sampleData.lock);
    memcpy(sampleData.historyBuffer, sampleData.currBuffer, sampleData.currBufferSize * sizeof(double));
    sampleData.historyBufferSize = sampleData.currBufferSize;
    sampleData.currBufferSize = 0;
    pthread_mutex_unlock(&sampleData.lock);
}

int Sampler_getHistorySize(void)
{
    pthread_mutex_lock(&sampleData.lock);
    int size = sampleData.historyBufferSize;
    pthread_mutex_unlock(&sampleData.lock);
    return size;
}

double* Sampler_getHistory(int *size)
{
    pthread_mutex_lock(&sampleData.lock);
    double * historyCopy = (double*) malloc(sampleData.historyBufferSize * sizeof(double));
    if (historyCopy != NULL) {
        memcpy(historyCopy, sampleData.historyBuffer, sampleData.historyBufferSize * sizeof(double));
    }
    *size = sampleData.historyBufferSize;
    pthread_mutex_unlock(&sampleData.lock);
    return historyCopy;
}


double Sampler_getAverageReading(void)
{
    pthread_mutex_lock(&sampleData.lock);
    double avg = sampleData.currentAvg;
    pthread_mutex_unlock(&sampleData.lock);
    return avg;
}

long long Sampler_getNumSamplesTaken(void) 
{
    pthread_mutex_lock(&sampleData.lock);
    long long total = sampleData.numSamplesTaken;
    pthread_mutex_unlock(&sampleData.lock);
    return total;
}

