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


#include <iostream>
#include <sstream>
#include <deque>

#include "stats.h"
#include "timer.h"

#include "trashconfig.h"
#include "twpath.h"
#include "osrm.h"
#include "move.h"
#include "vehicle.h"
#include "basevehicle.h"


/**
   prev curr next
   ttpc + serv(c) + ttcn
   inf when TWV
*/
double Vehicle::timePCN(POS prev, POS curr, POS next) const  {
#ifdef DOSTATS
 STATS->inc("Vehicle::timePCN positions ");
#endif
	if ( next==size() ) return path.timePCN(prev,curr,dumpSite);
	else return path.timePCN(prev,curr,next);
}

/* no tw checks */
double Vehicle::timePCN(Trashnode &prev, Trashnode &curr, Trashnode &next) const  {
#ifdef DOSTATS
 STATS->inc("Vehicle::timePCN nodes ");
#endif
	double time= prev.getTT(curr)+curr.getservicetime()+curr.getTT(next);
	return time;
}


/**

For a truck with n containers, \f$ testedMoves= n * (n +1) / 2\f$ 

if positive savings moves are found, those are added to moves 
otherwise all the negative savings moves are added to moves

if it happens that all moves generate TWC, in that case moves does not change
if \f$ n = 0 \f$ then moves does not change

return the number of moves added to moves
*/



long int Vehicle::eval_intraSwapMoveDumps( Moves &moves, int  truckPos,  double factor, const TWC<Trashnode> &twc ) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::eval_intraSwapMoveDumps ");
#endif

#ifndef TESTED
std::cout<<"Entering Vehicle::eval_intraSwapMoveDumps \n";
#endif
	if (path.size()==1) return 0;
	int fromPos,withPos;
        double newCost;
	double savings;
        double deltaTime;

        Vehicle truck = (*this);
	std::deque<Move>  negSavingsMoves;

        double originalCost= truck.getCost(twc);

	int originalMovesSize=moves.size();
	int deltaMovesSize=0;
	int otherNid;
	Move move;


    for (fromPos=1;fromPos<path.size()-1; fromPos++) {
	if (isdump(fromPos)) continue; //skiping dump
        Trashnode node = path[fromPos]; //saved for roll back
	for(withPos=fromPos+1; withPos<path.size();withPos++ ){
	  if (isdump(withPos)) continue; //skiping dump
	  otherNid=path[withPos].getnid();
          if ( truck.applyMoveIntraSw(fromPos,  withPos) ) { //move can be done
		newCost=truck.getCost(twc);
		savings= originalCost - newCost;
		truck=(*this);
                move.setIntraSwMove(truckPos, fromPos,  node.getnid(), withPos, otherNid, savings );
		if (savings>0) {
                  moves.insert(move);
		  deltaMovesSize++;
		} else negSavingsMoves.push_back(move);
	  } else truck=(*this);
	}
    }
    if ( deltaMovesSize ) return deltaMovesSize;
    moves.insert(negSavingsMoves.begin(),negSavingsMoves.end());
    return negSavingsMoves.size();
}

/*
    void setIntraSwMove( int fromTruck, int fromPos, int fromId, int withPos, int withId); 
*/

long int Vehicle::eval_interSwapMoveDumps( Moves &moves, const Vehicle &otherTruck,int  truckPos,int  otherTruckPos,  double factor,  const TWC<Trashnode> &twc ) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::eval_interSwapMoveDumps ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::eval_interSwapMoveDumps \n";
#endif
double minSavings=-5;
//        if ( path[fromPos].isdump() ) return moves.size();

//        Trashnode node = path[fromPos]; //saved for roll back
        Vehicle truck = (*this);
        Vehicle other = otherTruck;
	Trashnode tLast= path[size()-1];
	Trashnode oLast= other.path[other.path.size()-1];
	double truckDelta,otherDelta;
	int numNotFeasable=0;
bool inspect = (truckPos+otherTruckPos)==5 and (truckPos*otherTruckPos)==0; //inspect close combination 0,5
        double originalCost= truck.getCost(twc)  + other.getCost(twc);
        double originalDuration= truck.getduration()  + other.getduration();
        double newCost,savings,newDuration;
	int deltaMovesSize=0;
        int fromNodeId,toNodeId;
	Move move;

	int inc=5;
	for ( int m=1;m<6;m++) { 
        for ( int i=m; i<truck.size(); i+=inc) {
	   if (truck.path[i].isdump() ) continue;

	   fromNodeId=truck.path[i].getnid();
	   for ( int k=1; k<inc+1; k++) {
           for ( int j=k; j<other.size(); j+=inc) {
		if (other.path[j].isdump()) continue;
		if (numNotFeasable > factor * ( truck.getn() * other.getn()) ) {
   			std::cout<<"\n LEAVING WITH numNotFeasable"<<numNotFeasable;
   			std::cout<<"\n LEAVING WITH moves"<<deltaMovesSize;
			return deltaMovesSize;
		}
		if (deltaMovesSize > factor * ( truck.getn() * other.getn()) ) {
   			std::cout<<"\n LEAVING WITH moves"<<deltaMovesSize;
   			std::cout<<"\n LEAVING WITH numNotFeasable"<<numNotFeasable;
			return deltaMovesSize;
		}

		if (j==other.size()-1) {
   		    otherDelta=	 timePCN(other.path[j-1],truck.path[i],other.dumpSite)
  			- timePCN(other.path[j-1],other.path[j],other.dumpSite);
		} else {
   		    otherDelta= timePCN(other.path[j-1],truck.path[i],other.path[j+1])
			- timePCN(other.path[j-1],other.path[j],other.path[j+1]);
		}
		if (i==truck.size()-1) {
   		    truckDelta= timePCN(truck.path[i-1],other.path[j],truck.dumpSite)
   			- timePCN(truck.path[i-1],truck.path[i],truck.dumpSite);
		} else {
   		    truckDelta= timePCN(truck.path[i-1],other.path[j],truck.path[i+1])
   			- timePCN(truck.path[i-1],truck.path[i],truck.path[i+1]);
		}

	       //basic checking for time violation
	       if (other.dumpSite.deltaGeneratesTWV(otherDelta) 
		or other.endingSite.deltaGeneratesTWV(otherDelta)
		or other.path[other.size()-1].deltaGeneratesTWV(otherDelta) ) continue;  //Time Violation, not considered
	       if (truck.dumpSite.deltaGeneratesTWV(truckDelta) 
		or truck.endingSite.deltaGeneratesTWV(truckDelta)
		or truck.path[truck.size()-1].deltaGeneratesTWV(truckDelta) ) continue;  //Time Violation, not considered
/*
		if ((tLast.getArrivalTime()+truckDelta) > tLast.closes() 
		   or  (oLast.getArrivalTime()+otherDelta) > oLast.closes() 
		   or (minSavings > -(truckDelta+otherDelta)) ) {
			numNotFeasable++;
			continue;
		}
*/
#ifdef LOG
if (inspect) {
   std::cout<<" with"<<j<<"\t";other.path[j].dumpeval();
   //std::cout<<"old timePCN"<<timePCN(other.path[j-1],other.path[j],other.path[j+1])<<"\n";
   //std::cout<<"new timePCN"<<timePCN(other.path[j-1],truck.path[i],other.path[j+1])<<"\n";
   std::cout<<"delta timePCN"<<otherDelta<<"\n";
   std::cout<<"last container:"; other.path[other.path.size()-1].dumpeval();
   std::cout<<"last new arrival time"<<oLast.getArrivalTime()+otherDelta;

   //std::cout<<"\n\nnew timePCN"<<timePCN(truck.path[i-1],other.path[j],truck.path[i+1])<<"\n";
   //std::cout<<"old timePCN"<<timePCN(truck.path[i-1],truck.path[i],truck.path[i+1])<<"\n";
   std::cout<<"delta timePCN"<<truckDelta<<"\n";
   std::cout<<"last container:"; truck.path[truck.path.size()-1].dumpeval();
   std::cout<<"last new arrival time"<<tLast.getArrivalTime()+truckDelta;
   std::cout<<"\n";
   truck.tau(); other.tau(); 
}
#endif	
	        toNodeId=other.path[j].getnid();


		savings=_MIN();
		if ( truck.applyMoveInterSw(other, i, j)) {
		   newCost=truck.getCost(twc) + other.getCost(twc);
		   newDuration=truck.getduration() + other.getduration();
		   savings= originalCost - newCost;
    		   move.setInterSwMove( truckPos,  i,  fromNodeId,  otherTruckPos, j, toNodeId, savings); 
                   moves.insert(move);
                   deltaMovesSize++;
		} else numNotFeasable++;
 
		truck= (*this);
		other=otherTruck;
                if (savings>0 and inc!=1) {i=std::max(1,i-inc);inc=1;break;}
            }
	    }
        }
	}
	#ifdef LOG
   	std::cout<<"\n NORMAL WITH moves"<<deltaMovesSize;
   	std::cout<<"\n NORMAL WITH numNotFeasable"<<numNotFeasable;
	std::cout<<"\n limit was"<<(factor * ( getn() * otherTruck.getn()) ) <<"\n";
	#endif
	if ( deltaMovesSize ) return deltaMovesSize;
        return 0;
}





// space reserved for TODO list
bool Vehicle::e_insertIntoFeasableTruck(const Trashnode &node,int pos) {
#ifdef DOSTATS
 STATS->inc("Vehicle::e_insertIntoFeasableTruck ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::e_insertIntoFeasableTruck \n";
#endif
	assert( feasable() ); 
	double localCost=cost;
     	if ( not path.e__insert(node,pos,maxcapacity) ) {
	        assert( feasable() );
		return false;
	}
     	evalLast();

	if (not feasable() ) {
		path.e_remove(pos,maxcapacity);
     		evalLast();
		assert(localCost == cost);
	        assert( feasable() );
		return false;
        };

	assert( feasable() );
	return true;
}
/*
//dont forget, negative savings is a higher cost
bool Vehicle::eval_erase(int at, double &savings) const {
#ifdef DOSTATS
 STATS->inc(" ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::eval_erase \n";
#endif
	assert (at<size() and at>0 );
	if ( path[at].isdump() ) { savings=_MIN(); return false;}
	Vehicle truck = (*this);
	truck.path.erase(at);
	if ( not truck.e_makeFeasable(at) ) savings = _MIN(); // -infinity
        else savings = cost - truck.cost;

	return truck.feasable();
};
*/	
//dont forget, negative savings is a higher cost
bool Vehicle::eval_erase(int at, double &savings,const TWC<Trashnode> &twc) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::eval_erase ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::eval_erase \n";
#endif
        assert (at<size() and at>0 );
        if ( path[at].isdump() ) { savings=_MIN(); return false;}
        Vehicle truck = (*this);
	double oldcost=truck.getCost(twc);

        truck.path.erase(at);

        if ( not truck.e_makeFeasable(at) ) savings = _MIN(); // -infinity
        else savings = oldcost - truck.getCost(twc);

#ifdef TESTED
std::cout<<"ERASE : oldcost"<<oldcost<<"\tnewcost"<<truck.getCost(twc)<<"\tsavings"<<oldcost - truck.getCost(twc)<<"\n";
std::cout<<"\n";
#endif
        return truck.feasable();
};

long int Vehicle::eval_insertMoveDumps( const Trashnode &node,Moves &moves, int fromTruck, int fromPos, int toTruck, double eraseSavings, double factor, const TWC<Trashnode> &twc) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::eval_insertMoveDumps ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::eval_insertMoveDumps \n";
#endif
        Vehicle truck = (*this);
        std::deque<int> unTestedPos;
        std::deque<int> unfeasablePos;
        std::deque<int> impossiblePos;
        int currentPos,testingPos;
	double oldcost=truck.getCost(twc);
	double newcost;
	Move move;
#ifdef TESTED
truck.dumpCostValues();
#endif

        for ( int i=1; i<=size(); i++) unTestedPos.push_back(i);
        while (unTestedPos.size()) {
             currentPos=unTestedPos.back();
             unTestedPos.pop_back();
             truck.insert(node,currentPos);
	
             if ( not truck.e_makeFeasable(currentPos) ) {
#ifdef TESTED
truck.tau();
truck.dumpeval();
std::cout<<"\n";
assert(true==false);
#endif
                impossiblePos.push_back(currentPos);
                if ( path.size()*factor > impossiblePos.size() ) return moves.size();
             } else {
                assert ( truck.feasable() );
	        newcost=truck.getCost(twc);
#ifdef TESTED
std::cout<<"insert to "<<toTruck<<": oldcost"<<oldcost<<"\tnewcost"<<truck.getCost(twc)
	<<"\ninsert savings="<< (oldcost-newcost) <<"\teraseSavings"<<eraseSavings<<"\tsavings"<<oldcost - newcost + eraseSavings<<"\n";
std::cout<<"\n";
#endif
    		move.setInsMove( fromTruck, fromPos, node.getnid(), toTruck, currentPos, (cost-truck.cost + eraseSavings)    ); 
                moves.insert(move);
#ifdef TESTED
move.dump();
#endif
             }
             truck=(*this);
        }
        return moves.size();
}
	
	
/*
long int Vehicle::eval_insertMoveDumps( const Trashnode &node,std::deque<Move> &moves, int fromTruck, int fromPos, int toTruck, double eraseSavings, double factor) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::eval_insertMoveDumps ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::eval_insertMoveDumps \n";
#endif
	Vehicle truck = (*this);
	std::deque<int> unTestedPos;
	std::deque<int> unfeasablePos;
	std::deque<int> impossiblePos;
	int currentPos,testingPos;
	Move move;



        for ( int i=1; i<=size(); i++) unTestedPos.push_back(i); 
        while (unTestedPos.size()) {
             currentPos=unTestedPos.back();
	     unTestedPos.pop_back();
	     truck.insert(node,currentPos);
             if ( not truck.e_makeFeasable(currentPos) ) {
		impossiblePos.push_back(currentPos);
                if ( path.size()*factor > impossiblePos.size() ) return moves.size(); 
             } else {
		assert ( truck.feasable() );
    		move.setInsMove( fromTruck, fromPos, node.getnid(), toTruck, currentPos, (cost-truck.cost + eraseSavings)    ); 
		moves.push_back(move);

                truck.remove(currentPos);
		//unknown state of truck here
                while ( unTestedPos.size()>0 ) {
                   testingPos= unTestedPos.back();
	           unTestedPos.pop_back();
		   if ( testingPos<path.size() and  path[ testingPos ].isdump()) continue; //skipping dumps
        	   if ( truck.e_insertIntoFeasableTruck( node, testingPos) ) {
    			move.setInsMove( fromTruck, fromPos, node.getnid(), toTruck, testingPos, (cost-truck.cost + eraseSavings)    ); 
			moves.push_back(move);
                   } else unfeasablePos.push_back( testingPos);
		   truck.remove( testingPos );
		}
		unTestedPos=unfeasablePos;
             }
             truck=(*this);
        }
	return moves.size();
}
*/

bool Vehicle::e_makeFeasable(int currentPos) {
#ifdef DOSTATS
 STATS->inc(" Vehicle::e_makeFeasable ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::e_makeFeasable\n";
#endif
    path.e__adjustDumpsToMaxCapacity(currentPos, dumpSite, maxcapacity);
    evalLast();
    return feasable();
}

bool Vehicle::applyMoveINSerasePart(int nodeNid, int pos) {
#ifdef DOSTATS
 STATS->inc("Vehicle::applyMoveINSerasePart ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::applyMoveINSerasePart\n";
#endif
	assert (path[pos].getnid()==nodeNid); //if this assertion fails might be because its not being applied to the correct solution
	if (not (path[pos].getnid()==nodeNid))  return false;
        path.erase(pos);
        e_makeFeasable( pos );
if (not feasable() ) dumpeval();
        assert ( feasable() );
        return feasable();
}


bool Vehicle::applyMoveINSinsertPart(const Trashnode &node, int pos) {
#ifdef DOSTATS
 STATS->inc(" ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::applyMoveINSinsertPart\n";
#endif
	path.insert(node,pos);
	e_makeFeasable( pos );
if (not feasable() ) dumpeval();
	assert ( feasable() );
	return feasable();
}

bool Vehicle::applyMoveInterSw(Vehicle &otherTruck,int truckPos, int otherTruckPos) {
#ifdef DOSTATS
 STATS->inc("Vehicle::applyMoveInterSw ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::applyMoveIntraSw\n";
#endif

	path.swap( truckPos,  otherTruck.path, otherTruckPos);

        if (not e_makeFeasable( truckPos )) return false;
        if (not otherTruck.e_makeFeasable( otherTruckPos )) return false;

        //evalLast();
        //otherTruck.evalLast();

        assert ( feasable() );
        assert ( otherTruck.feasable() );
        return feasable() and otherTruck.feasable();
}

bool Vehicle::applyMoveIntraSw(int  fromPos, int withPos) {
#ifdef DOSTATS
 STATS->inc("Vehicle::applyMoveIntraSw ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::applyMoveInterSw\n";
#endif
        path.swap( fromPos,  withPos);
        if (not e_makeFeasable( std::min(fromPos-1,withPos) ) ) return false;
        //evalLast(); done in makeFeasable
        assert ( feasable() );
        return feasable() ;
}
	

/*
bool Vehicle::e_insertMoveDumps( const Trashnode &node, int at) {
	assert (at<=size());
//
//        path.insert(node,at);
//        path.e_moveDumps(at);
//
}
*/


// Very TIGHT insertion 
    // insertion will not be performed if 
    //      TV and CV are  generated 
    //  true- insertion was done
    //  false- not inserted 
bool Vehicle::e_insertSteadyDumpsTight(const Trashnode &node, int at){
#ifdef DOSTATS
 STATS->inc("Vehicle::e_insertSteadyDumpsTight ");
#endif
    assert ( at<=size() );
#ifndef TESTED
std::cout<<"Entering Vehicle::e_insertSteadyDumpsTight \n";
#endif


    if ( deltaCargoGeneratesCV(node,at) ) return false;
    if ( deltaTimeGeneratesTV(node,at) ) return false;
path[size()-1].dumpeval();
    if ( path.e_insert(node,at,maxcapacity) ) return false;
    evalLast();

    assert ( feasable() );
    return true;
};


// end space reserved for TODO list


bool Vehicle::e_insertDumpInPath( const Trashnode &lonelyNodeAfterDump ) {
#ifdef DOSTATS
 STATS->inc("Vehicle::e_insertDumpInPath ");
#endif
#ifndef TESTED
std::cout<<"Entering Vehicle::e_insertDumpInPath \n";
#endif
    //we arrived here because of CV
    if ( deltaTimeGeneratesTV( dumpSite,lonelyNodeAfterDump ) ) return false;
    Trashnode dump=dumpSite;
    dump.setDemand(-getcargo());
    path.e_push_back(dump,maxcapacity);
    path.e_push_back(lonelyNodeAfterDump,maxcapacity);
    evalLast();

    assert ( feasable() );
    return true;
};
    

    




//bool Vehicle::deltaCargoGeneratesCV_AUTO(const Trashnode &node, int pos) const { //position becomes important

bool Vehicle::deltaCargoGeneratesCV(const Trashnode &node, int pos) const { //position becomes important
#ifdef DOSTATS
 STATS->inc("Vehicle::deltaCargoGeneratesCV ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::deltaCargoGeneratesCV \n";
//std::cout<<getcargo()<<"+"<<node.getdemand()<<" ¿? " <<getmaxcapacity()<<" \n";
#endif
     //cycle until a dump is found
     int i;
     for (i=pos; i<size() and not isdump(i); i++) {};
     // two choices i points to a dump or i == size() 
     // in any case the i-1 node has the truck's cargo
#ifdef TESTED
path[i-1].dumpeval();
std::cout<<getCargo(i-1)<<"+"<<node.getdemand()<<" ¿? " <<getmaxcapacity()<<" \n";
#endif
     return  ( path[i-1].getcargo() + node.getdemand() > maxcapacity  ) ;
};




//////////// Delta Time generates TV
bool Vehicle::deltaTimeGeneratesTV(const Trashnode &dump, const Trashnode &node) const {
#ifdef DOSTATS
 STATS->inc(" Vehicle::deltaTimeGeneratesTV ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::deltaTimeGeneratesTV  ";
std::cout<<" (S 1 2 3 D E )  (S 1 2 3 D N D E)"<<path.getDeltaTimeAfterDump(dumpSite,node)<<" + "<< getduration()<<" ¿? "<<  endingSite.closes();
std::cout<<"\n";
#endif
     return  ( path.getDeltaTimeAfterDump(dumpSite,node) + getduration()  > endingSite.closes() ) ;
}





bool Vehicle::deltaTimeGeneratesTV(const Trashnode &node, int pos) const {
#ifdef DOSTATS
 STATS->inc("Vehicle::deltaTimeGeneratesTV ");
#endif
#ifdef TESTED
std::cout<<"Entering Vehicle::deltaTimeGeneratesTV \n";
if (pos>path.size()) std::cout<<"CANT work with this pos:"<<pos<<"\n";
std::cout<<"\n";
if (pos==path.size()) std::cout<<" (S 1 2 3 D E )  (S 1 2 3 N D E)" << path.getDeltaTime(node,dumpSite)<<" + "<< getduration()<<" ¿? "<<  endingSite.closes()<<"\n";
else std::cout<<" (S 1 2 3 D E )  (S 1 2 N 3 D E) "<< path.getDeltaTime(node,pos)<<" + "<< getduration()<<" ¿? "<<  endingSite.closes()<<"\n";
std::cout<<"\n";
endingSite.dump();
#endif
     assert(pos<=path.size());
     if (pos==path.size()) return path.getDeltaTime(node,dumpSite) + getduration()  > endingSite.closes();
     else return  ( path.getDeltaTime(node,pos) + getduration()  > endingSite.closes() ) ;
}
//////////////

