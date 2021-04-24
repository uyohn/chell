#include <stdio.h>
#include <time.h>

// return formated time
void format_time(char *output){
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    //sprintf(output, "[%d %d %d %d:%d:%d]",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    sprintf(output, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
}
