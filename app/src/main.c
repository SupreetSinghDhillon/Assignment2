#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal/lightSensor.h"
#include "sampler.h"
#include "periodTimer.h"

int main()
{
    int count = 0;
    Sampler_init();
    while (count < 10900000) {
        count++;
    }
    Sampler_cleanup();

    return 0;
}