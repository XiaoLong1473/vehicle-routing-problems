
#include <limits>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "trashproblem.h"

double TrashProblem::distance(int n1, int n2) const {
    return datanodes[n1].distance(datanodes[n2]);
}


void TrashProblem::loadproblem(std::string& file) {
    std::ifstream in( file.c_str() );
    std::string line;

    // initialize the extents
    extents[0] = std::numeric_limits<double>::max();
    extents[1] = std::numeric_limits<double>::max();
    extents[2] = std::numeric_limits<double>::min();
    extents[3] = std::numeric_limits<double>::min();

    // read the nodes
    int cnt = 0;
    while ( std::getline(in, line) ) {
        cnt++;
        // skip comment lines
        if (line[0] == '#') continue;

        Trashnode node( line );
        if (!node.isvalid())
            std::cout << "ERROR: line: " << cnt << ": " << line;

        // compute the extents as we load the data for plotting
        if (node.getx() < extents[0]) extents[0] = node.getx();
        if (node.gety() < extents[1]) extents[1] = node.gety();
        if (node.getx() > extents[2]) extents[2] = node.getx();
        if (node.gety() > extents[3]) extents[3] = node.gety();

        datanodes.push_back(node);

        if (node.ispickup())
            pickups.push_back(node.getnid());
        else if (node.isdepot())
            depots.push_back(node.getnid());
        else if (node.isdump())
            dumps.push_back(node.getnid());
    }

    in.close();

    // add a small buffer around the extents
    extents[0] -= (extents[2] - extents[0]) * 0.02;
    extents[2] += (extents[2] - extents[0]) * 0.02;
    extents[1] -= (extents[3] - extents[1]) * 0.02;
    extents[3] += (extents[3] - extents[1]) * 0.02;

    buildDistanceMatrix();

    for (int i=0; i<datanodes.size(); i++)
        setNodeDistances(datanodes[i]);
}


void TrashProblem::setNodeDistances(Trashnode& n) {
    double dist = -1.0;
    int nid = -1;
    double dist2 = -1.0;
    int nid2 = -1;

    if (n.isdepot()) {
        n.setdepotdist(n.getnid(), 0.0, -1, -1.0);
        for (int i=0; i<dumps.size(); i++) {
            double d = dMatrix[n.getnid()][dumps[i]];
            if (nid == -1 or d < dist) {
                dist = d;
                nid = dumps[i];
            }
        }
        n.setdumpdist(nid, dist);
    }
    else if (n.isdump()) {
        n.setdumpdist(n.getnid(), 0.0);
        for (int i=0; i<depots.size(); i++) {
            double d = dMatrix[n.getnid()][depots[i]];
            if (nid == -1 or d < dist) {
                dist = d;
                nid = depots[i];
            }
        }
        n.setdepotdist(nid, dist, -1, -1.0);
    }
    else if (n.ispickup()) {
        for (int i=0; i<dumps.size(); i++) {
            double d = dMatrix[n.getnid()][dumps[i]];
            if (nid == -1 or d < dist) {
                dist = d;
                nid = dumps[i];
            }
        }
        n.setdumpdist(nid, dist);

        nid = -1;
        for (int i=0; i<depots.size(); i++) {
            double d = dMatrix[n.getnid()][depots[i]];
            if (nid == -1 or d < dist) {
                dist2 = dist;
                nid2 = nid;
                dist = d;
                nid = depots[i];
            }
        }
        n.setdepotdist(nid, dist, nid2, dist2);
    }
}


void TrashProblem::buildDistanceMatrix() {
    dMatrix.clear();
    dMatrix.resize(datanodes.size());
    for (int i=0; i<datanodes.size(); i++) {
        dMatrix[i].clear();
        dMatrix[i].resize(datanodes.size());
        for (int j=0; j<datanodes.size(); j++) {
            dMatrix[i][j] = datanodes[i].distance(datanodes[j]);
        }
    }
}


std::string TrashProblem::solutionAsText() const {
    std::stringstream ss;;
    std::vector<int> s = solutionAsVector();
    for (int i=0; i<s.size(); i++) {
        if (i) ss << ",";
        ss << s[i];
    }
    return ss.str();
}


std::vector<int>  TrashProblem::solutionAsVector() const {
    std::vector<int> s;
    for (int i=0; i<fleet.size(); i++) {
        if (fleet[i].route.size() == 0) continue;
        for (int j=0; j<fleet[i].route.size(); j++) {
            s.push_back(fleet[i].route[j].getnid());
        }
        s.push_back(-1);
    }
    return s;
}


void TrashProblem::nearestNeighbor() {

}


void TrashProblem::nearestInsertion() {

}


void TrashProblem::farthestInsertion() {

}


void TrashProblem::assignmentSweep() {

}


void TrashProblem::opt_2opt() {

}

void TrashProblem::dumpDmatrix() const {
    std::cout << "--------- dMatrix ------------" << std::endl;
    for (int i=0; i<dMatrix.size(); i++) {
        for (int j=0; j<dMatrix[i].size(); j++) {
            std::cout << i << "\t" << j << "\t" << dMatrix[i][j] << std::endl;
        }
    }
}


void TrashProblem::dumpFleet() const {
    std::cout << "--------- Fleet ------------" << std::endl;
    for (int i=0; i<fleet.size(); i++)
        fleet[i].dump();
}


void TrashProblem::dumpdataNodes() const {
    std::cout << "--------- Nodes ------------" << std::endl;
    for (int i=0; i<datanodes.size(); i++)
        datanodes[i].dump();
}


void TrashProblem::dumpDepots() const {
    std::cout << "--------- Depots ------------" << std::endl;
    for (int i=0; i<depots.size(); i++)
        datanodes[depots[i]].dump();
}


void TrashProblem::dumpDumps() const {
    std::cout << "--------- Dumps ------------" << std::endl;
    for (int i=0; i<dumps.size(); i++)
        datanodes[dumps[i]].dump();
}


void TrashProblem::dumpPickups() const {
    std::cout << "--------- Pickups ------------" << std::endl;
    for (int i=0; i<pickups.size(); i++)
        datanodes[pickups[i]].dump();
}


void TrashProblem::dump() const {
    dumpDepots();
    dumpDumps();
    dumpPickups();
    dumpFleet();
    std::cout << "Solution: " << solutionAsText() << std::endl;
}

