#ifndef GLOBAL_ROUTING_HPP
#define GLOBAL_ROUTING_HPP

#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>
#include <time.h>
#include <climits>
#include <stdlib.h>
#include <string>
#include <queue>
#include <map>

#define up 0
#define down 1
#define left 2
#define right 3

using namespace std;

class Pin
{
public:
    int x, y;
    void set_pt(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
    Pin() {}
    ~Pin() {}
};

class Point
{
public:
    vector<Pin> nbr_pt;
    Pin parent;
    int x, y;
    int label;
    double congest_cost;
    int cost; //astar cost
    int wirelength;
    bool visit;

    void set_parent(int x, int y)
    {
        parent.x = x;
        parent.y = y;
    }

    Point()
    {
        parent.x = parent.y = -1; //source
        cost = 0;
        congest_cost = 1.0;
        visit = false;
        wirelength = 0;
        label = -1;
        nbr_pt.resize(4); //4 direction
    }
    ~Point() {}
};

class Net
{
public:
    int id, numpins;
    vector<Pin> terminal;     //terminals on net
    vector<int> retrace_path; //0:up, 1:down, 2:left, 3:right
    vector<Pin> coor_path;
    vector<vector<double>> hor_cost, ver_cost;
    int retimes;
    //vector<vector<Point>> cost_pt;

    Net()
    {
        retimes = 0;
    }
};

class Edge
{
public:
    int capacity;
    int demand;
    int cost;
    int history;

    int overflow()
    {
        return (demand - capacity > 0) ? demand - capacity : 0;
    }

    bool is_ovf()
    {
        return (demand - capacity > 0) ? true : false;
    }

    void update_cost()
    {
        if (this->is_ovf())
        {
            this->cost = this->overflow();
        }
    }

    Edge()
    {
        history = 0;
        demand = 0;
        cost = 0;
    }

    ~Edge() {}
};

class global_routing
{
public:
    int grid_width, grid_height;
    int total_ovf, max_ovf;
    int numnets;
    int vc, hc; //vertical and horizontal capacity
    bool complete_filling;
    vector<Net> net;
    vector<vector<Point>> grid_pt;
    vector<vector<vector<Point>>> net_gridpt;
    vector<vector<Edge>> hor_edge, ver_edge;

    void getfile(char *);
    void outputfile(char *);
    void construct_grid();
    void filling(Pin, Pin); //now end current_label
    void retrace(Pin, Pin, Net &);
    void compute_cost();          //compute edge cost
    void rip_up_reroute(clock_t); //clock
    bool rip_path(Net &);
    void rip_up_random_route(clock_t);
    void update_cost();
    void detour_route(clock_t);
    int compute_net_cost(const Net &);
    bool is_net_overflow(const Net &);
    bool check_boundry(const Pin &);
    void show_label();
    void reset_label();
    void route_path(Net &);
    void routing();
    vector<int> rip_up_ovf(); //return overflow net id
    void route_net(Net &);
    void astar_backtrace(Net &);
};

#endif
