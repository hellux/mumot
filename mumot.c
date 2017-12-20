#include "server.h"

int main(int argc, char* args[])
{
    struct compositor_handlers handlers;
    return run_server(&handlers);
}
