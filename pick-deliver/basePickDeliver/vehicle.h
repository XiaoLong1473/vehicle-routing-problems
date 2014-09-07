#ifndef VEHICLE_H
#define VEHICLE_H

#include <deque> 
#include <vector>
#include "twpath.h"
#include "order.h"
#include "dpnode.h"
#include "bucketn.h"


class Compatible;


class Vehicle:public BucketN {
  private:
    int  maxcapacity;   
//    Twpath<Dpnode> path;
    Dpnode backToDepot;
    double cost;     
    //deque<Order> orders;

    /* for evaluating the truck */
    int  curcapacity;   

    double w1;          // weight for duration in cost
    double w2;          // weight for TWV in cost
    double w3;          // weight for CV in cost


  public:

    // constructors

    Vehicle() {
        maxcapacity = 0;
        curcapacity = 0;
        cost        = 0;
        w1 = w2 = w3 = 1.0;
    };

   Vehicle(const Dpnode &_depot,double _maxcapacity) {
        backToDepot=_depot;
        maxcapacity=_maxcapacity;
        w1 = w2 = w3 = 1.0;
        push_back(_depot);
   };
   Twpath<Dpnode> getpath() const;


    // accessors
    int getmaxcapacity() const {return maxcapacity; };

    void push_back(const Dpnode &pathstop);
    void insert(const Dpnode &pathstop,int at);
    void remove(int at);
    void removeOrder( const Order &order);
    void removeOrder(int orderid);
    void removePickup(int orderid);
    void removeDelivery(int orderid);
    void swapstops(int i,int j);
    void swap(int i,int j);
    void move(int fromi,int toj);
    bool insertOrderAfterLastPickup(const Order &order, const Compatible &twc);
    void pushOrder(const Order &order);
    void pushPickup(const Order &order);
    void pushDelivery(const Order &order);
    void insertPickup(const Order &order, const int at);
    void insertDelivery(const Order &order, const int at);

    Dpnode& getnode(int at) {return path[at];};

    void dump() const ;
    void smalldump()const ;
    bool sameorder(int i,int j){return path[i].getoid()==path[j].getoid();}
    void erase() {path.resize(1);};
    void clean() {path.e_resize(1,maxcapacity); evalLast(); };

    /*algorithm spesific */
    void findBetterForward(int &bestI, int &bestJ);
    bool findImprovment(int i);
    void hillClimbOpt();
    int  findForwardImprovment(const int i,double &bestcost) ;
    double costBetterPickupBackward(int &bppos, int &bdpos);
    double findBestCostBackForw(const int oid,int &bppos,int &bdpos);
    int    findBetterDeliveryForward(const int ppos,const int dpos,double &bestcost);
    bool hasTrip() {return path.size()>1;};
    bool hasNodes() {return !path.empty();};

bool isEmpty() const;
bool isEmptyTruck() const;



/* evaluation */
    bool feasable() { return backToDepot.gettwvTot() == 0 and backToDepot.getcvTot() == 0;}
    bool hascv()const { return backToDepot.getcvTot() != 0;}
    bool hastwv()const { return backToDepot.gettwvTot() != 0;}

    void evaluate();
    void evalLast();
    int gettwvTot() const { return backToDepot.gettwvTot(); };
    int getcvTot() const { return backToDepot.getcvTot(); };
    int getcurcapacity() const { return curcapacity; };
    double getduration() const { return backToDepot.gettotDist(); };
    double getcost() const { return cost; };
    //double getcost(double w1,double w2,double w3);// { return   w1*D + w2*TWV + w3*CV; }

    double getw1() const { return w1; };
    double getw2() const { return w2; };
    double getw3() const { return w3; };

    // these should be const
    //double distancetodepot(int i) { return path[i].distance(path.getdepot()); };
    //double distancetodump(int i) { return path[i].distance(path.getdumpsite()); };


    // mutators
    void setweights(double _w1, double _w2, double _w3) {
        w1 = _w1;
        w2 = _w2;
        w3 = _w3;
    };

    void tau() const ;

    void plot(std::string file,std::string title,int carnumber);

};

#endif
