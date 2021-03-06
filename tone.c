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
#include "handle_ferr.h"

extern int optind;
static char *pname;

static void help(void)
{
    printf
        ("%s [--samples|-s samples/sec] [--freq|-f frequency] [--volume|-v percent] [--duration|-d milliseconds] [--out|-o file]\n",
         pname);
}

static void dump(FILE * fd, char *fname, float buf[], int len)
{
    int i;

    if (len == 0)
        return;

    for (i = 0; i < len; i++) {
        if (fprintf(fd, "%g\n", buf[i]) < 0) {
            handle_ferr(fname, pname);
            if ((fd != stdout) && (fd != stderr)) {
                fclose(fd);
            }

            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    pa_sample_spec ss = {
        .format = PA_SAMPLE_FLOAT32NE,
        .channels = 1
    };

    double t, v = 0.005, freq = 440.0;
    float buf[BUFSIZ];          /* float is 32 bits */
    pa_simple *s = NULL;
    FILE *out = NULL;
    char *fname;
    int ret = 1, error, srate = 44100, msecs = 500, c, i =
        0, lind, extra = 0, d = 0;
    size_t ctr;
    struct option longopts[] = {
        {"samples", 1, 0, 0},
        {"freq", 1, 0, 0},
        {"duration", 1, 0, 0},
        {"out", 1, 0, 0},
        {"volume", 1, 0, 0},
        {"help", 0, 0, 0},
        {0, 0, 0, 0}
    };

    pname = argv[0];

    while ((c =
            getopt_long(argc, argv, "s:f:d:o:v:h", longopts,
                        &lind)) != -1) {
        switch (c) {
        case 0:
            switch (lind) {
            case 0:
                srate = atoi(optarg);
                break;
            case 1:
                freq = strtod(optarg, NULL);
                break;
            case 2:
                msecs = atoi(optarg);
                break;
            case 3:
                fname = optarg;
                d++;
                break;
            case 4:
                v = strtod(optarg, NULL);
                break;
            case 5:
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
        case 'f':
            freq = strtod(optarg, NULL);
            break;
        case 'd':
            msecs = atoi(optarg);
            break;
        case 'o':
            fname = optarg;
            d++;
            break;
        case 'v':
            v = strtod(optarg, NULL);
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

    ss.rate = srate;

    ctr = (((double) msecs) / 1000.0) * srate;

    extra = ctr % BUFSIZ;

    if (d) {
        if ((out = fopen(fname, "w")) == NULL) {
            handle_ferr(pname, fname);
            goto finish;
        }
    }

    if (!
        (s =
         pa_simple_new(NULL, pname, PA_STREAM_PLAYBACK, NULL, "playback",
                       &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n",
                pa_strerror(error));
        goto finish;
    }

    for (t = 0;; t += 1.0 / srate) {
        if (ctr < BUFSIZ)
            break;

        buf[i++] = v * sin(2 * M_PI * freq * t);

        if (i == BUFSIZ) {
            if (pa_simple_write(s, buf, BUFSIZ * sizeof(*buf), &error) < 0) {
                fprintf(stderr,
                        __FILE__ ": pa_simple_write() failed: %s\n",
                        pa_strerror(error));
                goto finish;
            }

            if (d)
                dump(out, fname, buf, BUFSIZ);

            ctr -= BUFSIZ;
            i = 0;
        }
    }

    if (extra > 0) {
        for (i = 0; i < extra; i++, t += 1.0 / srate)
            buf[i] = v * sin(2 * M_PI * freq * t);

        if (pa_simple_write(s, buf, extra * sizeof(*buf), &error) < 0) {
            fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n",
                    pa_strerror(error));
            goto finish;
        }

        if (d)
            dump(out, fname, buf, extra);
    }

    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n",
                pa_strerror(error));
        goto finish;
    }

    ret = 0;

  finish:
    if (s)
        pa_simple_free(s);

    if (d && out && (out != stdout) && (out != stderr))
        fclose(out);

    exit(ret ? EXIT_FAILURE : EXIT_SUCCESS);
}
