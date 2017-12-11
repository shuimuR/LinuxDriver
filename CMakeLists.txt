cmake_minimum_required(VERSION 3.8)
project(LinuxDriver)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        01_FirstDrv/Driver/FirstDrv.c
        01_FirstDrv/Driver/FirstDrv.mod.c
        01_FirstDrv/Test/FirstDrvTest.c
        02_ScanKey/Driver/ScanKey.c
        02_ScanKey/Driver/ScanKey.mod.c
        02_ScanKey/Test/ScanKeyTest.c
        03_KeyIntr/Driver/KeyIntr.c
        03_KeyIntr/Driver/KeyIntr.mod.c
        03_KeyIntr/Test/KeyIntrTest.c
        04_KeyPoll/Driver/KeyIntr.c
        04_KeyPoll/Driver/KeyIntr.mod.c
        04_KeyPoll/Test/KeyIntrTest.c
        05_Signal/Signal1.c
        06_KeySignal/Driver/KeyIntr.c
        06_KeySignal/Driver/KeyIntr.mod.c
        06_KeySignal/Test/KeyIntrTest.c
        07_KeyForOne/Driver_Atom/KeyIntr.c
        07_KeyForOne/Driver_Atom/KeyIntr.mod.c
        07_KeyForOne/Driver_SemMutex/KeyIntr.c
        07_KeyForOne/Driver_SemMutex/KeyIntr.mod.c
        07_KeyForOne/DriverBlock/KeyIntr.c
        07_KeyForOne/DriverBlock/KeyIntr.mod.c
        07_KeyForOne/Test/KeyIntrTest.c
        07_KeyForOne/Test_Block/KeyIntrTest.c
        08_Improve/Driver/KeyIntr.c
        08_Improve/Driver/KeyIntr.mod.c
        08_Improve/Test/KeyIntrTest.c
        09_ButtonInput/ButtonInput.c
        09_ButtonInput/ButtonInput.mod.c
        10_LED_BUS_DEV/Driver/LED_Dev.c
        10_LED_BUS_DEV/Driver/LED_Dev.mod.c
        10_LED_BUS_DEV/Driver/LED_Drv.c
        10_LED_BUS_DEV/Driver/LED_Drv.mod.c
        10_LED_BUS_DEV/Test/LEDTest.c
        11_LCD/Driver/LCD_Drv.c
        temp_4412/01_LED/Tiny_4412_LED.c)

add_executable(LinuxDriver ${SOURCE_FILES})