#include "server.h"
#include <wlr/util/log.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);

    KronServer *server = kron_server_create();
    int ret = kron_server_run(server);
    kron_server_destroy(server);
    return ret;
}
