#include "hal/lightSensor.h"

#define LIGHT_SENSOR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"

#define REF_VOLTAGE 1.8
#define MAX_READING 4095

double getLightSensorReading(void)
{
    FILE *pFile = fopen(LIGHT_SENSOR_PATH, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open export file.\n");
        exit(1);
    }
    int lsReadingRaw;
    if (fscanf(pFile, "%d", &lsReadingRaw) == 1) {
        double voltageReading = ((double) lsReadingRaw / MAX_READING) * REF_VOLTAGE;
        fclose(pFile);
        return voltageReading;
    } else {
        perror("Error reading from file.");
        fclose(pFile);
        exit(1);
    }
}