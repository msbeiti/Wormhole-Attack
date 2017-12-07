/**
 * @file main.cc
 * @author Eugen Paul, Mohamad Sbeiti
 */

 
#include "Scheduler.h"

int main(int arg, char **args) {

    // initialize and start scheduler
    Scheduler *sch = new Scheduler();
    sch->start();

    delete sch;

    return 0;
}




