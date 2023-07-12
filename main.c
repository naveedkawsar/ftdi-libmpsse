#include <stdlib.h>
#include <stdio.h>
#include "ftd2xx.h"
#include "libmpsse_spi.h"

// Re-define typedefs from WinTypes to get rid of compiler warnings
//typedef unsigned long int uint32_t;

void print_and_quit(char cstring[]) {
    printf("%s\n", cstring);
    getc(stdin);
    exit(1);
}

int main(int argc, char **argv)
{
    Init_libMPSSE();

    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE channelInfo;
    FT_HANDLE handle;
    printf("Start\n");

    // check how many MPSSE channels are available
    uint32 channel_count = 0;
    status = SPI_GetNumChannels(&channel_count);
    if (status != FT_OK)
        print_and_quit("Error while checking the number of available MPSSE channels.");
    else if (channel_count < 1)
        print_and_quit("Error: no MPSSE channels are available.");

    printf("There are %d channels available.\n\n", channel_count);

    Cleanup_libMPSSE();
    return 0;
}