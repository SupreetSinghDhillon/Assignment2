#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal/lightSensor.h"
#include "sampler.h"
#include "periodTimer.h"

int main()
{
    long long count = 0;
    Sampler_init();
    while (count < 109000000) {
        count++;

    }
    double avg = Sampler_getAverageReading();
    printf("Avg: %f\n",avg);
    int hist = Sampler_getHistorySize();
    printf("history size %d", hist);
    long long total = Sampler_getNumSamplesTaken();
    printf("total: %lld", total);

    Sampler_cleanup();

    return 0;
}