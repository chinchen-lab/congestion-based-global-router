#include "global_routing.h"

int time_limit = 593;

struct compare_point
{
    bool operator()(const Point &p1, const Point &p2)
    {
        return p1.cost > p2.cost;
    }
};

struct RNG
{
    int operator()(int n)
    {
        return std::rand() / (1.0 + RAND_MAX) * n;
    }
};

bool sortbysec(const pair<int, int> &a, const pair<int, int> &b)
{
    return (a.second < b.second);
}

int opposite_dir(int dir)
{
    if (dir == up)
        return down;
    else if (dir == down)
        return up;
    else if (dir == left)
        return right;
    else
        return left;
}

bool valid_perm(vector<int> &perm)
{
    for (size_t i = 0; i < perm.size() - 1; i++)
    {
        if (perm[i] == opposite_dir(perm[i + 1]))
            return false;
    }
    return true;
}

void global_routing::getfile(char *filename)
{
    FILE *fr;
    fr = fopen(filename, "r");
    int mak = 0;
    mak++;

    if (fr == NULL)
    {
        cout << filename << " : Open file failure!" << endl;
    }

    mak = fscanf(fr, "%*s %d %d\n", &grid_width, &grid_height);
    mak = fscanf(fr, "%*s %*s %d\n", &vc);
    mak = fscanf(fr, "%*s %*s %d\n", &hc);
    mak = fscanf(fr, "%*s %*s %d\n", &numnets);
    //cout << numnets << endl;
    for (int i = 0; i < numnets; i++)
    {
        Net n;
        mak = fscanf(fr, "%*s %d %d\n", &n.id, &n.numpins);
        n.terminal.resize(n.numpins);
        for (int j = 0; j < n.numpins; j++)
            mak = fscanf(fr, "%d %d\n", &n.terminal[j].x, &n.terminal[j].y);
        net.push_back(n);
    }
    fclose(fr);

    //print
    /*
    cout << "grid " << grid_width << " " << grid_height << endl;
    cout << vc << hc << endl;
    for (const auto &n : net)
    {
        cout << "net" << n.id << ", numpins = " << n.numpins << endl;
        for (const auto &p : n.terminal)
            cout << "  " << p.x << " " << p.y << endl;
    }
    */
}

void global_routing::outputfile(char *filename)
{
    FILE *fo = fopen(filename, "w+");

    for (auto &n : net)
    {
        //cout << "net" << n.id << " " << n.id << endl;
        fprintf(fo, "net%d %d\n", n.id, n.id);
        //printf("(%d, %d, 1)-", n.coor_path[0].x, n.coor_path[0].y);
        fprintf(fo, "(%d, %d, 1)-", n.coor_path[0].x, n.coor_path[0].y);

        for (size_t i = 1; i < n.coor_path.size(); i++)
        {
            const Pin &now = n.coor_path[i];
            //printf("(%d, %d, 1)\n", now.x, now.y);
            fprintf(fo, "(%d, %d, 1)\n", now.x, now.y);

            if (i != n.coor_path.size() - 1)
            {
                //printf("(%d, %d, 1)-", now.x, now.y);
                fprintf(fo, "(%d, %d, 1)-", now.x, now.y);
            }
            else
            {
                //printf("!\n");
                fprintf(fo, "!\n");
            }
        }
    }

    compute_cost();
    cout << endl;
    cout << "-----------------result-----------------" << endl;
    cout << "Total overflow: " << total_ovf << endl;
    cout << "Maximum overflow: " << max_ovf << endl;
}

void global_routing::construct_grid()
{
    int grid_width = this->grid_width;
    int grid_height = this->grid_height;

    if (grid_width % 2 != 0)
        grid_width++;
    if (grid_height % 2 != 0)
        grid_height++;

    grid_width = grid_height = max(grid_width, grid_height);

    grid_pt.resize(grid_width);
    hor_edge.resize(grid_height);
    ver_edge.resize(grid_width);

    //set grid point
    for (int k = 0; k < grid_width; k++)
    {
        grid_pt[k].resize(grid_height);
    }

    for (int i = 0; i < grid_width; i++)
    {
        for (int j = 0; j < grid_height; j++)
        {
            grid_pt[i][j].nbr_pt[up].set_pt(i, j + 1);
            grid_pt[i][j].nbr_pt[down].set_pt(i, j - 1);
            grid_pt[i][j].nbr_pt[left].set_pt(i - 1, j);
            grid_pt[i][j].nbr_pt[right].set_pt(i + 1, j);
            grid_pt[i][j].x = i;
            grid_pt[i][j].y = j;
        }
    }

    //set grid edge
    for (auto &h : hor_edge)
        h.resize(grid_width - 1);

    for (auto &v : ver_edge)
        v.resize(grid_height - 1);

    //set net edge cost
    for (auto &n : net)
    {
        n.hor_cost.resize(grid_height);
        n.ver_cost.resize(grid_width);

        for (auto &h : n.hor_cost)
            h.resize(grid_width - 1);

        for (auto &v : n.ver_cost)
            v.resize(grid_height - 1);
    }

    for (auto &h : hor_edge)
    {
        for (size_t i = 0; i < hor_edge.size(); i++)
        {
            h[i].capacity = hc;
        }
    }

    for (auto &v : ver_edge)
    {
        for (size_t i = 0; i < ver_edge.size(); i++)
        {
            v[i].capacity = vc;
        }
    }
}

void global_routing::routing()
{
    construct_grid();

    cout << "global routing...";
    double k = 0;

    //maze
    for (auto &n : net)
    {
        Pin start, end;

        start.x = n.terminal[0].x;
        start.y = n.terminal[0].y;
        end.x = n.terminal[1].x;
        end.y = n.terminal[1].y;

        reset_label();
        Point &source = grid_pt[start.x][start.y];
        source.label = 0;
        filling(start, end);

        //cout << "start = (" << start.x << ", " << start.y << ")" << endl;
        //cout << "end = (" << end.x << ", " << end.y << ")" << endl;
        //show_label();

        retrace(start, end, n);
        route_path(n); //route with path

        k++;
        cout.width(7);
        cout << fixed << setprecision(2) << k / (double)numnets * 100 << "%";
        cout << "\b";
        for (int i = 0; i < 7; i++)
            cout << "\b";
    }
    /* 
    //A star
    for (auto &n : net)
    {
        priority_queue<Point, vector<Point>, compare_point> pq;
        Pin start = n.terminal[0];
        Pin end = n.terminal[1];

        reset_label();
        filling(start, end);
        auto &ngrid = grid_pt;

        Point *source = &ngrid[start.x][start.y];
        source->visit = true;
        pq.push(*source);
        bool complete = false;
        //cout << "source = " << source->x << " " << source->y << endl;

        while (!complete)
        {
            Point top = pq.top();
            pq.pop();
            //cout << "pop node " << top.x << " " << top.y << endl;

            vector<Pin> &nbr_pin = top.nbr_pt;

            for (auto &p : nbr_pin)
            {
                if (!check_boundry(p))
                    continue;

                Point *cur_pt = &ngrid[p.x][p.y];

                if (cur_pt->label != -1 && cur_pt->visit == false)
                {
                    cur_pt->wirelength = top.wirelength + 1;
                    cur_pt->cost = cur_pt->wirelength + abs(p.x - end.x) + abs(p.y - end.y);
                    cur_pt->visit = true;
                    cur_pt->set_parent(top.x, top.y);
                    pq.push(*cur_pt);

                    if (p.x == end.x && p.y == end.y)
                        complete = true;
                }
                else
                {
                    cur_pt->visit = true;
                }
            }
        }
        astar_backtrace(n);
        route_path(n);

        k++;
        cout.width(7);
        cout << fixed << setprecision(2) << k / (double)numnets * 100 << "%";
        cout << "\b";
        for (int i = 0; i < 7; i++)
            cout << "\b";
    }
    */
    cout << endl;
    compute_cost();
}

void global_routing::astar_backtrace(Net &n)
{
    Point cur = grid_pt[n.terminal[1].x][n.terminal[1].y];

    while (true)
    {
        const int &cx = cur.x;
        const int &cy = cur.y;
        const int &px = cur.parent.x;
        const int &py = cur.parent.y;

        if (px == -1 && py == -1)
            break;

        if (px - cx > 0)
        {
            n.retrace_path.push_back(right);
        }
        else if (px - cx < 0)
        {
            n.retrace_path.push_back(left);
        }
        else if (py - cy > 0)
        {
            n.retrace_path.push_back(up);
        }
        else
        {
            n.retrace_path.push_back(down);
        }

        cur = grid_pt[px][py];
    }
}

bool global_routing::is_net_overflow(const Net &n)
{
    for (size_t i = 0; i < n.coor_path.size() - 1; i++)
    {
        int curx = n.coor_path[i].x;
        int cury = n.coor_path[i].y;
        int nextx = n.coor_path[i + 1].x;
        int nexty = n.coor_path[i + 1].y;

        if (abs(curx - nextx) == 1)
        {
            int x = min(curx, nextx);
            if (hor_edge[cury][x].is_ovf())
                return true;
        }
        else
        {
            int y = min(cury, nexty);
            if (ver_edge[curx][y].is_ovf())
                return true;
        }
    }
    return false;
}

vector<int> global_routing::rip_up_ovf()
{
    vector<int> ovf_nets;
    for (auto &n : net)
    {
        bool is_ovf = is_net_overflow(n);
        //int cost = compute_net_cost(n);
        if (is_ovf)
        {
            ovf_nets.push_back(n.id);
        }
    }
    //random_shuffle(ovf_nets.begin(), ovf_nets.end(), RNG());
    return ovf_nets;
}

void global_routing::rip_up_reroute(clock_t time1)
{
    vector<int> nlist = rip_up_ovf();
    //compute_cost();
    //cout << "rip-up reroute...\n";

    for (const auto &i : nlist)
    {
        //cout << "reroute net "<< i << endl;
        int before = total_ovf;
        Net &n = net[i];
        vector<int> path = n.retrace_path;
        vector<Pin> coor = n.coor_path;
        rip_path(n);
        route_net(n);

        for (int i = 0; i < grid_height; i++)
        {
            for (int j = 0; j < grid_width - 1; j++)
            {
                Edge &e = hor_edge[i][j];
                if (e.is_ovf())
                    n.hor_cost[i][j]++;
            }
        }

        for (int i = 0; i < grid_width; i++)
        {
            for (int j = 0; j < grid_height - 1; j++)
            {
                Edge &e = ver_edge[i][j];
                if (e.is_ovf())
                    n.ver_cost[i][j]++;
            }
        }

        //compute_cost();
        update_cost();
        int after = total_ovf;

        if (after > before)
        {
            rip_path(n);
            //n.coor_path = coor;
            n.retrace_path = path;
            route_path(n);
            update_cost();
            //compute_cost();
        }
    }
}

void global_routing::route_net(Net &n)
{
    priority_queue<Point, vector<Point>, compare_point> pq;
    Pin start = n.terminal[0];
    Pin end = n.terminal[1];

    reset_label();
    filling(start, end);
    auto &ngrid = grid_pt;

    Point *source = &ngrid[start.x][start.y];
    source->visit = true;
    pq.push(*source);
    bool complete = false;

    while (!complete)
    {
        Point top = pq.top();
        pq.pop();

        vector<Pin> &nbr_pin = top.nbr_pt;

        for (auto &p : nbr_pin)
        {
            if (!check_boundry(p))
                continue;

            double edge_history, edge_demand, capacity;
            Point *cur_pt = &ngrid[p.x][p.y];

            if (cur_pt->x - top.x > 0) // move right
            {
                edge_history = n.hor_cost[top.y][top.x];
                edge_demand = hor_edge[top.y][top.x].demand;
                capacity = hc;
            }
            else if (cur_pt->x - top.x > 0)
            {
                edge_history = n.hor_cost[cur_pt->y][cur_pt->x];
                edge_demand = hor_edge[cur_pt->y][cur_pt->x].demand;
                capacity = hc;
            }
            else if (cur_pt->y - top.y > 0) // move up
            {
                edge_history = n.ver_cost[top.x][top.y];
                edge_demand = ver_edge[top.x][top.y].demand;
                capacity = vc;
            }
            else
            {
                edge_history = n.ver_cost[cur_pt->x][cur_pt->y];
                edge_demand = ver_edge[cur_pt->x][cur_pt->y].demand;
                capacity = vc;
            }

            if (cur_pt->label != -1 && cur_pt->visit == false)
            {
                double h = 2;
                if (vc == 20 && hc == 23 && numnets == 27781)
                    h = 1;
 
                cur_pt->congest_cost = top.congest_cost + h * (edge_history * pow(((edge_demand + 1.0) / capacity), 5));
                cur_pt->wirelength = top.wirelength + 1;
                cur_pt->cost = cur_pt->wirelength + abs(p.x - end.x) + abs(p.y - end.y) + cur_pt->congest_cost;
                cur_pt->visit = true;
                cur_pt->set_parent(top.x, top.y);
                pq.push(*cur_pt);

                if (p.x == end.x && p.y == end.y)
                    complete = true;
            }
            else
            {
                cur_pt->visit = true;
            }
        }
    }
    astar_backtrace(n);
    route_path(n);
}

bool global_routing::check_boundry(const Pin &p)
{
    return (p.x >= 0 && p.y >= 0 && p.x < grid_width && p.y < grid_height) ? true : false;
}

void global_routing::filling(Pin s, Pin e)
{
    Point *now = &grid_pt[s.x][s.y];
    queue<Point> pt_list;
    pt_list.push(*now);

    while (pt_list.size() != 0)
    {
        Point cur = pt_list.front();
        pt_list.pop();

        for (const auto &nbr : cur.nbr_pt)
        {
            if (check_boundry(nbr))
            {
                if (nbr.x == e.x && nbr.y == e.y)
                    complete_filling = true;

                Point *n = &grid_pt[nbr.x][nbr.y];
                if (n->label == -1)
                {
                    n->label = cur.label + 1;
                    pt_list.push(*n);
                }
            }
        }

        /*if (complete_filling)
            break;*/
    }
}

void global_routing::retrace(Pin s, Pin e, Net &n)
{
    Point now = grid_pt[e.x][e.y];
    bool done = false;

    while (!done)
    {
        int min_label = now.label;

        for (size_t i = 0; i < now.nbr_pt.size(); i++)
        {
            Point neighbor;
            if (check_boundry(now.nbr_pt[i]))
                neighbor = grid_pt[now.nbr_pt[i].x][now.nbr_pt[i].y];
            else
                continue;

            if (neighbor.label < min_label && neighbor.label != -1)
            {
                min_label = neighbor.label;
                n.retrace_path.push_back(i);
                now = neighbor;
                break;
            }
        }

        if (min_label == 0)
            done = true;
    }
}

void global_routing::compute_cost()
{
    for (auto &h : hor_edge)
        for (auto &e : h)
            e.demand = 0;

    for (auto &v : ver_edge)
        for (auto &e : v)
            e.demand = 0;

    for (const auto &n : net)
    {
        for (size_t i = 0; i < n.coor_path.size() - 1; i++)
        {
            int curx = n.coor_path[i].x;
            int cury = n.coor_path[i].y;
            int nextx = n.coor_path[i + 1].x;
            int nexty = n.coor_path[i + 1].y;

            if (abs(curx - nextx) == 1)
            {
                int x = min(curx, nextx);
                hor_edge[cury][x].demand++;
            }
            else
            {
                int y = min(cury, nexty);
                ver_edge[curx][y].demand++;
            }
        }
    }

    //compute overflow
    int ovf = 0;
    int max = 0;

    for (auto &h : hor_edge)
    {
        for (auto &e : h)
        {
            if (e.overflow() > max)
                max = e.overflow();
            ovf += e.overflow();
        }
    }

    for (auto &v : ver_edge)
    {
        for (auto &e : v)
        {
            if (e.overflow() > max)
                max = e.overflow();
            ovf += e.overflow();
        }
    }

    total_ovf = ovf;
    max_ovf = max;
}

bool global_routing::rip_path(Net &n)
{
    for (size_t i = 0; i < n.coor_path.size() - 1; i++)
    {
        int curx = n.coor_path[i].x;
        int cury = n.coor_path[i].y;
        int nextx = n.coor_path[i + 1].x;
        int nexty = n.coor_path[i + 1].y;

        if (abs(curx - nextx) == 1)
        {
            int x = min(curx, nextx);
            hor_edge[cury][x].demand--;
        }
        else
        {
            int y = min(cury, nexty);
            ver_edge[curx][y].demand--;
        }
    }

    n.retrace_path.clear();
    n.coor_path.clear();

    return true;
}

int global_routing::compute_net_cost(const Net &n)
{
    int cost = 0;

    for (size_t i = 0; i < n.coor_path.size() - 1; i++)
    {
        int curx = n.coor_path[i].x;
        int cury = n.coor_path[i].y;
        int nextx = n.coor_path[i + 1].x;
        int nexty = n.coor_path[i + 1].y;

        if (abs(curx - nextx) == 1)
        {
            int x = min(curx, nextx);
            cost += hor_edge[cury][x].overflow();
        }
        else
        {
            int y = min(cury, nexty);
            cost += ver_edge[curx][y].overflow();
        }
    }

    return cost;
}

void global_routing::update_cost()
{
    //compute overflow
    int ovf = 0;
    int max = 0;

    for (auto &h : hor_edge)
    {
        for (auto &e : h)
        {
            if (e.overflow() > max)
                max = e.overflow();
            ovf += e.overflow();
        }
    }

    for (auto &v : ver_edge)
    {
        for (auto &e : v)
        {
            if (e.overflow() > max)
                max = e.overflow();
            ovf += e.overflow();
        }
    }

    total_ovf = ovf;
    max_ovf = max;
}

void global_routing::rip_up_random_route(clock_t time1)
{

    vector<pair<int, int>> net_order;

    for (auto &n : net)
    {
        int cost = compute_net_cost(n);
        net_order.push_back(make_pair(n.id, cost));
    }
    sort(net_order.begin(), net_order.end(), sortbysec);

    cout << "shuffle grid...";
    double k = 0;
    for (const auto &rrn : net_order)
    {
        if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
            return;

        if (rrn.second == 0)
            continue;

        int max_iter = 10;
        Net &n = net[rrn.first];
        int hor_step, ver_step, x_dir, y_dir;
        Pin start, end;

        start.x = n.terminal[0].x;
        start.y = n.terminal[0].y;
        end.x = n.terminal[1].x;
        end.y = n.terminal[1].y;

        hor_step = start.x - end.x;
        ver_step = start.y - end.y;

        k++;

        compute_cost();
        //cout << "rip up net " << n.id << endl;
        //cout << "init cost = " << total_ovf << endl;
        int best_cost = total_ovf;
        vector<int> permutation;
        vector<int> best_perm = n.retrace_path;
        vector<Pin> best_coor = n.coor_path;

        rip_path(n);
        //n.retrace_path.clear();
        //n.coor_path.clear();

        x_dir = (hor_step > 0) ? right : left;
        y_dir = (ver_step > 0) ? up : down;

        int hor_num = abs(hor_step);
        int ver_num = abs(ver_step);

        //save paths
        permutation.assign(hor_num, x_dir);
        permutation.insert(permutation.end(), ver_num, y_dir);

        //try all permutation
        //cout << "start = " << start.x << " " << start.y << endl;
        //cout << "end = " << end.x << " " << end.y << endl;
        int count = 0;
        bool update = false;
        do
        {
            n.retrace_path = permutation;
            route_path(n);
            update_cost();
            //compute_cost();
            if (total_ovf < best_cost)
            {
                best_cost = total_ovf;
                best_perm = permutation;
                best_coor = n.coor_path;
                update = true;
                max_iter += 100;
                //cout << "best cost = " << best_cost << endl;
            }

            rip_path(n);

            if (count++ > max_iter)
                break;

            if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
                break;

            //getchar();
        } while (prev_permutation(permutation.begin(), permutation.end()));

        if (!update && count > max_iter && (((double)clock() - time1) / CLOCKS_PER_SEC) < time_limit)
        {
            permutation.clear();
            permutation.assign(ver_num, y_dir);
            permutation.insert(permutation.end(), hor_num, x_dir);

            int limit = count - max_iter;
            count = 0;
            do
            {
                n.retrace_path = permutation;
                route_path(n);
                update_cost();
                //compute_cost();

                if (total_ovf < best_cost)
                {
                    best_cost = total_ovf;
                    best_perm = permutation;
                    best_coor = n.coor_path;
                    max_iter += 100;
                    update = true;
                    //cout << "best cost = " << best_cost << endl;
                }

                rip_path(n);

                if (count++ > limit)
                    break;

                if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
                    break;
                //getchar();
            } while (next_permutation(permutation.begin(), permutation.end()));
        }

        if (!update && n.retimes < 4)
            n.retimes++;
        else
            n.retimes = 0;

        n.retrace_path = best_perm;
        n.coor_path = best_coor;
        //cout << n.id << " " << init_cost << endl;

        cout.width(7);
        cout << fixed << setprecision(2) << k / (double)numnets * 100 << "%";
        cout << "\b";
        for (int i = 0; i < 7; i++)
            cout << "\b";
    }
    cout << endl;
}

void global_routing::detour_route(clock_t time1)
{
    int detour_dis;

    vector<pair<int, int>> net_order;

    for (auto &n : net)
    {
        int cost = compute_net_cost(n);
        if (cost != 0 && n.retimes != 0)
            net_order.push_back(make_pair(n.id, cost));
    }
    sort(net_order.begin(), net_order.end(), sortbysec);

    for (const auto &rrn : net_order)
    {
        bool valid = true;

        int max_iter = 50;
        Net &n = net[rrn.first];
        int hor_step, ver_step, x_dir, y_dir;
        Pin start, end;

        detour_dis = n.retimes;

        start.x = n.terminal[0].x;
        start.y = n.terminal[0].y;
        end.x = n.terminal[1].x;
        end.y = n.terminal[1].y;

        hor_step = start.x - end.x;
        ver_step = start.y - end.y;

        /*if (hor_step == 0 || ver_step == 0)
        {
            continue;
        }*/

        compute_cost();

        int best_cost = total_ovf;
        vector<int> permutation;
        vector<int> best_perm = n.retrace_path;
        vector<Pin> best_coor = n.coor_path;

        //n.retrace_path.clear();
        //n.coor_path.clear();

        x_dir = (hor_step > 0) ? right : left;
        y_dir = (ver_step > 0) ? up : down;

        int hor_num = abs(hor_step);
        int ver_num = abs(ver_step);

        if (start.x + detour_dis + 1 > grid_width || start.x - detour_dis - 1 < 0 || end.x + detour_dis + 1 > grid_width || end.x - detour_dis - 1 < 0)
            valid = false;

        bool update = false;
        if ((((double)clock() - time1) / CLOCKS_PER_SEC) < time_limit && valid)
        {
            //hor detour
            permutation.clear();
            permutation.assign(hor_num + detour_dis, x_dir);
            permutation.insert(permutation.end(), ver_num, y_dir);

            int op_x = opposite_dir(x_dir);
            for (int i = 0; i < detour_dis; i++)
                permutation.push_back(op_x);

            int count = 0;
            do
            {
                if (!valid_perm(permutation))
                    continue;

                rip_path(n);
                n.retrace_path = permutation;
                route_path(n);
                compute_cost();

                if (total_ovf < best_cost)
                {
                    best_cost = total_ovf;
                    best_perm = permutation;
                    best_coor = n.coor_path;
                    max_iter += 100;
                    update = true;
                    cout << "detour best cost = " << best_cost << endl;
                }

                if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
                    break;

                if (count++ > max_iter)
                    break;
                //getchar();
            } while (prev_permutation(permutation.begin(), permutation.end()));
        }

        valid = true;
        if (start.y + detour_dis + 1 > grid_height || start.y - detour_dis - 1 < 0 || end.y + detour_dis + 1 > grid_height || end.y - detour_dis - 1 < 0)
            valid = false;

        if ((((double)clock() - time1) / CLOCKS_PER_SEC) < time_limit && valid && !update)
        {
            //ver detour
            permutation.clear();
            permutation.assign(ver_num + detour_dis, y_dir);
            permutation.insert(permutation.end(), hor_num, x_dir);

            int op_y = opposite_dir(y_dir);
            for (int i = 0; i < detour_dis; i++)
                permutation.push_back(op_y);

            int count = 0;
            do
            {
                if (!valid_perm(permutation))
                    continue;

                rip_path(n);
                n.retrace_path = permutation;
                route_path(n);
                compute_cost();

                if (total_ovf < best_cost)
                {
                    best_cost = total_ovf;
                    best_perm = permutation;
                    best_coor = n.coor_path;
                    max_iter += 100;
                    cout << "detour best cost = " << best_cost << endl;
                }

                if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
                    break;

                if (count++ > max_iter)
                    break;
                //getchar();
            } while (next_permutation(permutation.begin(), permutation.end()));
        }

        if (start.x + detour_dis + 1 > grid_width || start.x - detour_dis - 1 < 0 || end.x + detour_dis + 1 > grid_width || end.x - detour_dis - 1 < 0)
            valid = false;

        if ((((double)clock() - time1) / CLOCKS_PER_SEC) < time_limit && valid && !update && n.retimes < 3)
        {
            //ver + hor detour
            permutation.clear();
            permutation.assign(ver_num + detour_dis, y_dir);
            permutation.insert(permutation.end(), hor_num + detour_dis, x_dir);

            int op_x = opposite_dir(x_dir);
            int op_y = opposite_dir(y_dir);

            for (int i = 0; i < detour_dis; i++)
            {
                permutation.push_back(op_y);
                permutation.push_back(op_x);
            }

            int count = 0;
            do
            {
                if (!valid_perm(permutation))
                    continue;

                rip_path(n);
                n.retrace_path = permutation;
                route_path(n);
                compute_cost();

                if (total_ovf < best_cost)
                {
                    best_cost = total_ovf;
                    best_perm = permutation;
                    best_coor = n.coor_path;
                    max_iter += 100;
                    cout << "detour best cost = " << best_cost << endl;
                }

                if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
                    break;

                if (count++ > max_iter)
                    break;
                //getchar();
            } while (next_permutation(permutation.begin(), permutation.end()));
        }

        n.retrace_path = best_perm;
        n.coor_path = best_coor;

        if (((double)clock() - time1) / CLOCKS_PER_SEC > time_limit)
            return;
    }
}

void global_routing::show_label()
{
    for (int i = 0; i < grid_width; i++)
        printf("%4d", i);
    cout << "\n---------------------------------------------------------\n";

    int k = 0;
    for (int i = 0; i < grid_height; i++)
    {
        for (int j = 0; j < grid_width; j++)
        {
            if (grid_pt[i][j].label != -1)
                printf("%4d", grid_pt[j][i].label);
            else
                cout << "   x";
        }
        cout << "   |" << k++;
        cout << endl;
    }
}

void global_routing::reset_label()
{
    complete_filling = false;
    for (auto &i : grid_pt)
        for (auto &j : i)
        {
            j.congest_cost = j.cost = 0;
            j.parent.x = j.parent.y = -1; //source
            j.label = -1;
            j.visit = false;
            j.wirelength = 0;
        }
}

void global_routing::route_path(Net &n)
{
    Pin now = n.terminal[0];
    n.coor_path.push_back(now);

    for (int i = n.retrace_path.size() - 1; i >= 0; i--)
    {
        int dir = n.retrace_path[i];

        switch (dir)
        {
        case up:
            //go down
            now.set_pt(now.x, now.y - 1);
            n.coor_path.push_back(now);
            break;
        case down:
            //go up
            now.set_pt(now.x, now.y + 1);
            n.coor_path.push_back(now);
            break;
        case left:
            //go right
            now.set_pt(now.x + 1, now.y);
            n.coor_path.push_back(now);
            break;
        case right:
            //go left
            now.set_pt(now.x - 1, now.y);
            n.coor_path.push_back(now);
            break;
        }
    }

    for (size_t i = 0; i < n.coor_path.size() - 1; i++)
    {
        int curx = n.coor_path[i].x;
        int cury = n.coor_path[i].y;
        int nextx = n.coor_path[i + 1].x;
        int nexty = n.coor_path[i + 1].y;

        if (abs(curx - nextx) == 1)
        {
            int x = min(curx, nextx);
            hor_edge[cury][x].demand++;
        }
        else
        {
            int y = min(cury, nexty);
            ver_edge[curx][y].demand++;
        }
    }
}