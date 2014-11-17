/*VRP*********************************************************************
 *
 * vehicle routing problems
 *      A collection of C++ classes for developing VRP solutions
 *      and specific solutions developed using these classes.
 *
 * Copyright 2014 Stephen Woodbridge <woodbri@imaptools.com>
 * Copyright 2014 Vicky Vergara <vicky_vergara@hotmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the MIT License. Please file LICENSE for details.
 *
 ********************************************************************VRP*/

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

#include <stdio.h>

#ifdef DOSTATS
#include "timer.h"
#include "stats.h"
#endif

#include "trashconfig.h"
#include "node.h"
#include "osrmclient.h"

#ifdef WITHOSRM
#include "osrm.h"
#endif

#include "twnode.h"
#include "trashnode.h"
#include "twpath.h"
#include "feasableSolLoop.h"
#include "tabuopt.h"


void Usage() {
    std::cout << "Usage: trash file (no extension)\n";
}

static std::string font = "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf";


int main(int argc, char **argv) {

    if (argc < 2) {
        Usage();
        return 1;
    }

    std::string infile = argv[1];

    // MUST call this once to initial communications via cURL
    //cURLpp::Cleanup myCleanup;

    try {
	OsrmClient oc;
	oc.testOsrmClient();

	#ifdef DOSTATS
        Timer starttime;
	#endif

        #ifdef PLOT
        CONFIG->set("plotDir", "./logs/");
	#endif
        CONFIG->dump("CONFIG");

       
        FeasableSolLoop tp(infile);

	#ifndef LOG
        tp.dump();
        std::cout << "FeasableSol time: " << starttime.duration() << std::endl;
	#endif
	#ifdef DOSTATS
        STATS->set("zzFeasableSol time", starttime.duration());
	#endif

        tp.setInitialValues();
        tp.computeCosts();
	#ifdef DOSTATS
        STATS->set("zInitial cost", tp.getCost());
        STATS->set("yNode count", tp.getNodeCount());
        STATS->set("yVehicle count", tp.getFleetSize());

        Timer searchtime;
	#endif

        TabuOpt ts(tp);
        ts.setMaxIteration(1000);
        ts.search();

	#ifdef DOSTATS
        STATS->set("zzSearch time", searchtime.duration());
	#endif

        Solution best = ts.getBestSolution();
        best.computeCosts();

	#ifdef DOSTATS
        STATS->set("zzTotal time", starttime.duration());
	#endif

	#ifndef LOG
        best.dump();
	#endif

	#ifdef DOSTATS
        STATS->set("zBest cost", best.getCost());
        STATS->set("zBest distance", best.getDistance());

        STATS->dump("Final");
	#endif

    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}




