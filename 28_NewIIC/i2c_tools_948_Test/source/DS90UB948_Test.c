#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "TestPattern.h"
#include "i2c-dev.h"

int main(int argc, char **argv)
{
    Open_DS90UB948();
    DS90UB948_ReadID();
    TestPatternInit();
    while(1)
    {
        sleep(1000);
    }
    return 0;
}
