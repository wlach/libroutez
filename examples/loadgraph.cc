#include <stdlib.h>
#include "tripgraph.h"

using namespace std;
using namespace boost;


int main(int argc, char *argv[])
{
    // this example does nothing other than simply load a graph into memory
    // useful for profiling memory usage

    if (argc < 2)
    {
        printf("Usage: %s <graph file> ", argv[0]);
        return 1;
    }

    printf("Loading graph...\n");
    TripGraph g;
    g.load(argv[1]);

    return 0;
}
