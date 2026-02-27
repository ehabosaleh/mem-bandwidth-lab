#include"../include/pipe_bench.h"

int main(int argc, char **argv) {
    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;     
    int warmup = 100;

    for (int i=1;i<argc;i++) {
        if (strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i] + 8);
        else if (strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i] + 9);
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
        } else {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            usage(argv[0]);
        }
    }


    int p2c[2], c2p[2];
    if (pipe(p2c) != 0 || pipe(c2p) != 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // child: read from p2c[0], write to c2p[1]
        close(p2c[1]);
        close(c2p[0]);
        child_loop(p2c[0], c2p[1]);
        close(p2c[0]);
        close(c2p[1]);
        _exit(0);
    }

    // parent
    close(p2c[0]);
    close(c2p[1]);

    printf("Pipe benchmark (parent<->child). min=%zu max=%zu iters=%d\n", min_bytes, max_bytes, iters);
    printf("%-20s%-20s%-20s\n","Bytes","latency_us","bandwidth_MBps\n");

    for (int s=min_bytes; s<max_bytes; s*=2) {
        size_t msg=s;
        
        int lat_iters=iters;
        int lat_warm=warmup;

        double lat_us=0.0;
        double lat_s=measure_latency_one(msg, lat_iters, lat_warm, p2c[1], c2p[0], -1, -1);
        lat_us = lat_s*1e6;

        double bw = (msg /(lat_us*1e-6))/(1024.0*1024.0);
        printf("%-20zu %-20.3f %-20.3f\n", msg, lat_us,bw);
        fflush(stdout);
    }

    unsigned char op='Q';
    write_all(p2c[1],&op,1);

    close(p2c[1]);
    close(c2p[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    return 0;
}
