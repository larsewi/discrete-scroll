#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <ApplicationServices/ApplicationServices.h>

#define SIGN(x) (((x) > 0) - ((x) < 0))
#define LINES 2

CGEventRef cgEventCallback(__attribute__((unused)) CGEventTapProxy proxy, __attribute__((unused)) CGEventType type, CGEventRef event, __attribute__((unused)) void *refcon)
{
    if (!CGEventGetIntegerValueField(event, kCGScrollWheelEventIsContinuous)) {
        int64_t delta = CGEventGetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis1);

        CGEventSetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1, SIGN(delta) * LINES);
    }

    return event;
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid > 0)
    {
        return EXIT_SUCCESS;
    }

    if (setsid() < 0)
    {
        perror("setsid");
        return EXIT_FAILURE;
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid > 0)
    {
        return EXIT_SUCCESS;
    }

    umask(0);

    chdir("/");
    int x;
    for (x = (int) sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close(x);
    }

    openlog("scrolld", LOG_PID, LOG_DAEMON);

    syslog(LOG_NOTICE, PACKAGE_NAME " started");

    CFMachPortRef eventTap;
    CFRunLoopSourceRef runLoopSource;

    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, 1 << kCGEventScrollWheel, cgEventCallback, NULL);
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    CFRunLoopRun();

    CFRelease(eventTap);
    CFRelease(runLoopSource);

    syslog(LOG_NOTICE, "scrolld terminated");
    closelog();

    return 0;
}
