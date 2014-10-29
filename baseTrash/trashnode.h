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
#ifndef TRASHNODE_H
#define TRASHNODE_H

#include "tweval.h"

class Trashnode : public Tweval {
  protected:

  public:
    // accessors
//    void dump() const;
    void dumpeval() const;

    // state
    bool isdepot() const {return type==0;};
    bool isStarting() const {return type==0;};
    bool isdump() const {return type==1;};
    bool ispickup() const {return type==2;};
    bool isEnding() const {return type==3;};
    bool isvalid() const;

    // mutators
//    void setvalues(int _nid, double _x, double _y, int _demand,
//                   int _tw_open, int _tw_close, int _service, int _ntype);
//    void setntype(int _ntype) { ntype = _ntype; };

//Constructors
    Trashnode(std::string line);
    ~Trashnode() {};
    Trashnode() : Tweval() { }; 
    Trashnode(int _id, double _x, double _y, int _open, int _close, int _service, int _demand, int _sid) : Tweval(_id, _x, _y, _open, _close, _service, _demand, _sid) {};


//  OLD IDEAS DOWN BELLOW ARE COMMENTED OUT
protected:
    double depotdist;       // distance to nearest depot
    long int depotnid;      // nid of the closet depot
    double depotdist2;      // distance to nearest depot
    long int depotnid2;     // nid of the closet depot
    double dumpdist;        // distance to nearest dump
    long int dumpnid;       // nid of closet dump
public:
//accessors
    double getdepotdist() const {return depotdist;};
    long int getdepotnid() const {return depotnid;};
    double getdepotdist2() const {return depotdist2;};
    long int getdepotnid2() const {return depotnid2;};
    double getdumpdist() const {return dumpdist;};
    long int getdumpnid() const {return dumpnid;};

//mutators
    void setdepotdist(int _nid, double _dist, int _nid2, double _dist2);
    void setdumpdist(int _nid, double _dist);
//END OF OLD IDEAS

};

#endif
