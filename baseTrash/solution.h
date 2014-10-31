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
#ifndef SOLUTION_H
#define SOLUTION_H

#include <deque>
#include <cmath>

#include "prob_trash.h"
#include "twbucket.h"
#include "twpath.h"
#include "pg_types_vrp.h"
#include "vehicle.h"
#include "plot.h"
#include "move.h"

const double EPSILON = 0.001;

class Solution: public Prob_trash {
  protected:
typedef  TwBucket<Trashnode> Bucket;

    std::deque<Vehicle> fleet;


    double totalDistance;
    double totalCost;
    double w1,w2,w3;

  public:

    Solution(const Prob_trash& P):Prob_trash(P){}; 
    Solution(const std::string &infile, const std::vector<int> &solution);


    void setweights(double _w1,double _w2,double _w3) {w1=_w1;w2=_w2;w3=_w3;};
    void dumproutes();
    void tau() ;
    void plot(std::string file,std::string title);
    std::string solutionAsText() const ;
    std::string solutionAsTextID() const ;
    std::vector<int>  solutionAsVector() const ;
    std::vector<int>  solutionAsVectorID() const ;

    vehicle_path_t *getSolutionForPg(int& count) const;

    void computeCosts();
    double getCost() const ;
    double getDistance() const ;
    int getFleetSize() const { return fleet.size(); };
    double getAverageRouteDurationLength();

    Solution& operator=( const Solution& rhs ) {
        if ( this != &rhs ) {
            totalDistance = rhs.totalDistance;
            totalCost = rhs.totalCost;
            fleet = rhs.fleet;
        }
        return *this;
    };

    bool operator == (Solution &another) const {
        return fleet.size() == another.fleet.size() &&
               std::abs(totalCost - another.totalCost) < EPSILON;
    };

   bool solutionEquivalent (Solution &another)  {
        computeCosts();
        another.computeCosts();
        return fleet.size() == another.fleet.size() &&
               std::abs(totalCost - another.totalCost) < EPSILON;

    };

    bool operator <  (Solution &another) const {
        return fleet.size() < another.fleet.size() || totalCost < another.totalCost;
    };


bool applyInsMove( const Move &move) ;
bool applyInterSwMove( const Move &move) ;
bool applyIntraSwMove( const Move &move) ;







// Cost related

    int v_computeCosts();

	void dumpCostValues() {
		for (int i=0;i<fleet.size();i++) 
			fleet[i].getcost(twc);
		for (int i=0;i<fleet.size();i++) 
			fleet[i].dumpCostValues();
	}

	void setInitialValues() {
		for (int i=0;i<fleet.size();i++) 
			fleet[i].setInitialValues(C,twc,pickups);
	}

// code moved from OLD CODE TO BE INTEGRATED
bool feasable() const;
double getduration() const ;
double getcost() const ;
int getTWV() const ;
int getCV() const ;

void dump() const;
void dumpFleet() const ;
void dumpSummary() const ;

};

#endif

