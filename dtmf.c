#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <pulse/simple.h>
#include <pulse/error.h>

extern int optind;
static char *pname;

static void help(void)
{
    printf
        ("%s [--samples|-s samples/sec] [--duration|-d milliseconds] KEYS\n",
         pname);
    puts("duration is the duration of each tone, KEYS are some sequence of the keys 0-9A-D*#");
    puts("samples defaults to 44100, duration defaults to 500 msec");
}

int main(int argc, char *argv[])
{
    pa_sample_spec ss = {
        .format = PA_SAMPLE_FLOAT32NE,
        .channels = 1
    };

    double t;
    float buf[BUFSIZ];          /* float is 32 bits */
    pa_simple *s = NULL;
    int ret = 1, srate = 44100, msecs =
        500, freq1, freq2, error, lind, extra, c, i, j, k;
    size_t ctr, ct;
    struct option longopts[] = {
        {"samples", 1, 0, 0},
        {"duration", 1, 0, 0},
        {"help", 0, 0, 0},
        {0, 0, 0, 0}
    };

    pname = argv[0];

    while ((c = getopt_long(argc, argv, "s:d:h", longopts, &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                srate = atoi(optarg);
                break;
            case 1:
                msecs = atoi(optarg);
                break;
            case 2:
                help();
                exit(EXIT_SUCCESS);
                break;
            default:
                help();
                exit(EXIT_FAILURE);
                break;
            }

            break;

        case 's':
            srate = atoi(optarg);
            break;
        case 'd':
            msecs = atoi(optarg);
            break;
        case 'h':
            help();
            exit(EXIT_SUCCESS);
            break;
        default:
            help();
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "no keys specified\n");
        help();
        goto finish;
    }

    ss.rate = srate;

    ctr = (((double) msecs) / 1000.0) * srate;

    extra = ctr % BUFSIZ;

    if (!
        (s =
         pa_simple_new(NULL, pname, PA_STREAM_PLAYBACK, NULL, "playback",
                       &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n",
                pa_strerror(error));
        goto finish;
    }

    for (j = 0; argv[optind][j] != '\0'; j++) {
        k = argv[optind][j];

        switch (k) {
        case '1':
            freq1 = 697;
            freq2 = 1209;
            break;
        case '2':
            freq1 = 697;
            freq2 = 1336;
            break;
        case '3':
            freq1 = 697;
            freq2 = 1477;
            break;
        case 'A':
            freq1 = 697;
            freq2 = 1633;
            break;
        case '4':
            freq1 = 770;
            freq2 = 1209;
            break;
        case '5':
            freq1 = 770;
            freq2 = 1336;
            break;
        case '6':
            freq1 = 770;
            freq2 = 1477;
            break;
        case 'B':
            freq1 = 770;
            freq2 = 1633;
            break;
        case '7':
            freq1 = 852;
            freq2 = 1209;
            break;
        case '8':
            freq1 = 852;
            freq2 = 1336;
            break;
        case '9':
            freq1 = 852;
            freq2 = 1477;
            break;
        case 'C':
            freq1 = 852;
            freq2 = 1633;
            break;
        case '*':
            freq1 = 941;
            freq2 = 1209;
            break;
        case '0':
            freq1 = 941;
            freq2 = 1336;
            break;
        case '#':
            freq1 = 941;
            freq2 = 1477;
            break;
        case 'D':
            freq1 = 941;
            freq2 = 1633;
            break;
        default:
            continue;
            break;
        }

        ct = ctr;
        i = 0;

        for (t = 0;; t += 1.0 / srate) {
            if (ct < BUFSIZ)
                break;

            buf[i++] =
                sin(2 * M_PI * freq1 * t) + sin(2 * M_PI * freq2 * t);

            if (i == BUFSIZ) {
                if (pa_simple_write(s, buf, BUFSIZ * sizeof(*buf), &error)
                    < 0) {
                    fprintf(stderr,
                            __FILE__ ": pa_simple_write() failed: %s\n",
                            pa_strerror(error));
                    goto finish;
                }

                ct -= BUFSIZ;
                i = 0;
            }
        }

        if (extra > 0) {
            for (i = 0; i < extra; i++, t += 1.0 / srate)
                buf[i] =
                    sin(2 * M_PI * freq1 * t) + sin(2 * M_PI * freq2 * t);

            if (pa_simple_write(s, buf, extra * sizeof(*buf), &error) < 0) {
                fprintf(stderr,
                        __FILE__ ": pa_simple_write() failed: %s\n",
                        pa_strerror(error));
                goto finish;
            }
        }

        if (pa_simple_drain(s, &error) < 0) {
            fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n",
                    pa_strerror(error));
            goto finish;
        }
    }

    ret = 0;

  finish:
    if (s)
        pa_simple_free(s);

    exit(ret ? EXIT_FAILURE : EXIT_SUCCESS);
}
