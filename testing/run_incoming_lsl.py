# Auxiliary script to provide a marker lsl stream
import time

import pylsl


def init_lsl() -> pylsl.StreamOutlet:
    # Create a marker stream info
    # Make sure the name is the same as in src/main.c:
    # const char INLET_STREAM_NAME[] = "MyParadigmIncoming";
    info = pylsl.StreamInfo(
        "MyParadigmIncoming", "Markers", 1, 0, "string", "myuiauxstream"
    )
    return pylsl.StreamOutlet(info)


def main(delay_s: float = 1, cat_every: int = 10):
    outlet = init_lsl()
    print("Sending markers every", delay_s, "seconds")
    i = 0
    while True:
        if i % cat_every == 0:
            outlet.push_sample(["cat"])
        else:
            outlet.push_sample([f"Marker{i}"])
        time.sleep(delay_s)
        i += 1


if __name__ == "__main__":
    main()
