#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef AF_INET
#include <sys/socket.h>
#endif
#ifndef INADDR_ANY
#include <netinet/in.h>
#endif

#include <algorithm>
#include <cassert>
#include <cstdio>

#include "bitbang_jtag.hpp"

#if 1
#  define D(x) x
#else
#  define D(x)
#endif

BitbangJTAG::BitbangJTAG(uint16_t port, bool *tck, bool *trstn, bool *tms, bool *tdi, bool *tdo) :
    tck(tck),
    trstn(trstn),
    tms(tms),
    tdi(tdi),
    tdo(tdo),
    socket_fd(0),
    client_fd(0)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "remote_bitbang failed to make socket: %s (%d)\n", strerror(errno), errno);
        abort();
    }

    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    int reuseaddr = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        fprintf(stderr, "remote_bitbang failed setsockopt: %s (%d)\n", strerror(errno), errno);
        abort();
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "remote_bitbang failed to bind socket: %s (%d)\n", strerror(errno), errno);
        abort();
    }

    if (listen(socket_fd, 1) == -1) {
        fprintf(stderr, "remote_bitbang failed to listen on socket: %s (%d)\n", strerror(errno), errno);
        abort();
    }

    socklen_t addrlen = sizeof(addr);
    if (getsockname(socket_fd, (struct sockaddr *) &addr, &addrlen) == -1) {
        fprintf(stderr, "remote_bitbang getsockname failed: %s (%d)\n", strerror(errno), errno);
        abort();
    }

    printf("Listening for remote bitbang connection on port %d.\n", ntohs(addr.sin_port));
    fflush(stdout);
}

void BitbangJTAG::accept() {
    client_fd = ::accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
        if (errno == EAGAIN) {
            // No client waiting to connect right now.
        } 
        else {
            fprintf(stderr, "failed to accept on socket: %s (%d)\n", strerror(errno), errno);
            abort();
        }
    }
    else {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }
}

void BitbangJTAG::tick() {
    if (!is_connected())
        return;
    
    char recv_chr = -1;
    char send_chr = -1;
    
    // read char from port
    int n = read(client_fd, &recv_chr, 1);
    // if (n == -1) {
    //     fprintf(stderr, "remote_bitbang failed to read from client: %s (%d)\n", strerror(errno), errno);
    //     abort();
    // }

    if (recv_chr != -1) {
        printf("rbb: '%c'\n", recv_chr);
        switch (recv_chr) {
            case 'B': printf("** BLINK ON **\n"); break;
            case 'b': printf("** BLINK OFF **\n"); break;
            case 'r': reset(); break;
            case '0': set_pins(0, 0, 0); break;
            case '1': set_pins(0, 0, 1); break;
            case '2': set_pins(0, 1, 0); break;
            case '3': set_pins(0, 1, 1); break;
            case '4': set_pins(1, 0, 0); break;
            case '5': set_pins(1, 0, 1); break;
            case '6': set_pins(1, 1, 0); break;
            case '7': set_pins(1, 1, 1); break;
            case 'R': send_chr = get_tdo() ? '1' : '0'; break;
            case 'Q': quit = true; break;
            default:
                    fprintf(stderr, "remote_bitbang got unsupported command '%c'\n", recv_chr);
        }
    }

    // send chr back to client
    if (send_chr != -1) {
        n = write(client_fd, &send_chr, 1);
        if (n == -1) {
            fprintf(stderr, "remote_bitbang failed to write to client: %s (%d)\n", strerror(errno), errno);
            abort();
        }
    }   
}

