#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool ones_only = false;
static bool discrete_only = false;
static uint64_t event_count = 0;
static CGEventTimestamp first_timestamp = 0;
static CGEventTimestamp previous_timestamp = 0;

static CGEventRef handle_event(CGEventTapProxy proxy, CGEventType type, CGEventRef event,
                               void *user_info) {
    (void)proxy;
    (void)user_info;

    if (type != kCGEventScrollWheel) {
        return event;
    }

    bool continuous =
        CGEventGetIntegerValueField(event, kCGScrollWheelEventIsContinuous) != 0;
    if (discrete_only && continuous) {
        return event;
    }

    event_count++;

    if (ones_only) {
        puts("1");
        return event;
    }

    CGEventTimestamp timestamp = CGEventGetTimestamp(event);
    if (first_timestamp == 0) {
        first_timestamp = timestamp;
    }

    double elapsed_ms = (double)(timestamp - first_timestamp) / 1000000.0;
    int64_t vertical = CGEventGetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis1);
    int64_t horizontal = CGEventGetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis2);
    if (previous_timestamp == 0) {
        printf("1  #%04" PRIu64
               "  t=%9.1f ms  dt=      - ms  vertical=%4" PRId64
               "  horizontal=%4" PRId64 "  continuous=%s\n",
               event_count, elapsed_ms, vertical, horizontal, continuous ? "yes" : "no");
    } else {
        double interval_ms = (double)(timestamp - previous_timestamp) / 1000000.0;
        printf("1  #%04" PRIu64
               "  t=%9.1f ms  dt=%7.1f ms  vertical=%4" PRId64
               "  horizontal=%4" PRId64 "  continuous=%s\n",
               event_count, elapsed_ms, interval_ms, vertical, horizontal,
               continuous ? "yes" : "no");
    }

    previous_timestamp = timestamp;
    return event;
}

static void print_usage(const char *program_name) {
    printf("Usage: %s [--discrete-only] [--ones]\n\n", program_name);
    puts("Watches scroll-wheel events received by macOS.\n");
    puts("  default  Print a marker, sequence number, timing, and scroll delta.");
    puts("  --discrete-only  Ignore continuous events (not suitable for this ZMK encoder).");
    puts("  --ones   Print only `1` for each received event.\n");
    puts("Stop with Control-C. Avoid using a trackpad or mouse while measuring.");
}

int main(int argc, char *argv[]) {
    for (int index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--ones") == 0) {
            ones_only = true;
        } else if (strcmp(argv[index], "--discrete-only") == 0) {
            discrete_only = true;
        } else if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[index]);
            fprintf(stderr, "Run with --help to see available options.\n");
            return 1;
        }
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    CGEventMask event_mask = CGEventMaskBit(kCGEventScrollWheel);
    CFMachPortRef event_tap = CGEventTapCreate(
        kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionListenOnly, event_mask,
        handle_event, NULL);

    if (event_tap == NULL) {
        fputs("Failed to monitor input events.\n", stderr);
        fputs("Allow your terminal in System Settings > Privacy & Security > Accessibility, "
              "then run this script again.\n",
              stderr);
        return 1;
    }

    CFRunLoopSourceRef source =
        CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, true);

    if (!ones_only) {
        puts("Monitoring scroll events. Rotate the encoder; stop with Control-C.");
        puts("Each line beginning with `1` is one event received by macOS.");
    }

    CFRunLoopRun();

    CFRelease(source);
    CFRelease(event_tap);
    return 0;
}
