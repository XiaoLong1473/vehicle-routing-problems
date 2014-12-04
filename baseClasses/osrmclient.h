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
#ifndef VRP_OSRMCLIENT_H
#define VRP_OSRMCLIENT_H

#include "DataStructures/Coordinate.h"
#include "Server/DataStructures/RouteParameters.h"
//#include "Library/OSRM.h"

#include "logger.h"
#include "node.h"

#include <string>
#include <deque>
#include <vector>

// load our assert to throw macros and tell rapidjson to use them
#include "vrp_assert.h"
#define RAPIDJSON_ASSERT assert
#include <rapidjson/document.h>

#include "timer.h"
#include "stats.h"

/*! \class OsrmClient
 * \brief This class provides a shared memory connection to OSRM.
 *
 * This class interfaces with OSRM via a shared memory connection and wraps
 * the interface into a simple class to abstract the features we need
 * access to. This interface is approximately 50 time faster than using
 * the URL based interface.
 *
 * \todo This iterface style receives OSRM results as json text documents
 *       and we have to parse them. I might be worth the effort to dig deeper
 *       into the OSRM code to avoid this step.
 */

class OSRM;
class OsrmClient {

  private:

    RouteParameters route_parameters;   ///< The OSRM request structure
    int status;                         ///< Current state of the object
    std::string
    err_msg;                ///< An error message if an error is reported.
    std::string httpContent;            ///< the json response document
    static bool
    connectionAvailable;           ///< once set to false, it doesnt try to make a connection
    static OSRM  *routing_machine;
    static OsrmClient *p_osrm;
    OsrmClient();
    OsrmClient( const OsrmClient &other ) {};
    OsrmClient &operator=( const OsrmClient & ) {};
    bool use, addPenalty;

  public:
    static OsrmClient *Instance() {
        if ( !p_osrm ) // Only allow one instance of class to be generated.
            p_osrm = new OsrmClient;

        return p_osrm;
    }

    void clear();
    void addViaPoint( double lat, double lon );
    void addViaPoint( const Node &node );
    void addViaPoints( const std::deque<Node> &path );

    /*!
     * \brief Set whether you want the path geometry returned.
     *
     * This should be left as False because it is much faster it you
     * do not need the geometry. It defaults to false.
     *
     * \param[in] want True or False if you want the geometry returned.
     */
    void setWantGeometry( bool want ) { route_parameters.geometry = want; };
    void usePenalty( bool desition ) { addPenalty = desition; };
    bool getPenalty() const { return addPenalty; };
    void useOsrm( bool desition ) { use = desition; };
    bool getUse( ) const { return use; };
    bool getOsrmViaroute();
    bool getOsrmTime( double lat1, double lon1 , double lat2, double lon2,
                      double &time );
    bool getOsrmTime( double lat1, double lon1 , double lat2, double lon2,
                      const std::string &hint1, const std::string &hint2, double &time );
    bool getOsrmTime( const Node &node1, const Node &node2, double &time );
    bool getOsrmTime( const Node &node1, const Node &node2, const Node &node3,
                      double &time );
    bool getOsrmTime( const Node &node1, const Node &node2, const Node &node3,
                      const Node &node4, double &time );
    bool getOsrmTime( double &time );
    bool getOsrmGeometry( std::deque<Node> &geom );
    bool getOsrmHints( std::deque<std::string> &hints );
    int getStatus() const { return status; };
    std::string getErrorMsg() const { return err_msg; };
    bool testOsrmClient();

  private:
    bool getTime( rapidjson::Document &jtree, double &time );
    bool getGeom( rapidjson::Document &jtree, std::deque<Node> &geom );
    bool getHints( rapidjson::Document &jtree, std::deque<std::string> &hints );
    bool getOsrmPenalty( double &penalty );
    bool getPenalty( rapidjson::Document &jtree, double &penalty );

  public:
    void dump() {
        DLOG( INFO ) << "----- OsrmClient ----------"
                     << "\nstatus: " << status
                     << "\nerr_msg: " << err_msg
                     << "\ncoordinates.size(): " << route_parameters.coordinates.size()
                     << "\nhttpContent: " << httpContent;
    };
};

#define osrm OsrmClient::Instance()

#endif
