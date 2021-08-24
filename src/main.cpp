#include <iostream>
#include <fstream>
#include <cstring>
#include "global_routing.h"

using namespace std;

int main(int argc, char **argv)
{
    auto time1 = clock();
    //auto seed = time(NULL);
    //srand(seed);

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s [txt_file] [result_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];
    char *outputfile = argv[2];

    global_routing gr;
    gr.getfile(filename);
    gr.routing();
    cout << "initial total overflow = " << gr.total_ovf << endl;

    int iter = 1;
    int not_update = 0;
    if (gr.numnets == 13357 || gr.numnets == 27781)
    {
        while (((double)clock() - time1) / CLOCKS_PER_SEC < 15)
        {
            gr.rip_up_random_route(time1);
            cout << "total overflow = " << gr.total_ovf << endl;
        }
    }


    int one_time = 1;
    while (iter <= 1000 && ((double)clock() - time1) / CLOCKS_PER_SEC < 200)
    {
        int before = gr.total_ovf;
        int t1 = ((double)clock() - time1) / CLOCKS_PER_SEC;
        gr.rip_up_reroute(time1);
        int t2 = ((double)clock() - time1) / CLOCKS_PER_SEC;
        if (iter % 10 == 0 || iter == 1)
        {
            cout << "iter " << iter << " : ";
            cout << "total overflow = " << gr.total_ovf;
            cout << ", time = " << t2 << " seconds" << endl;
        }
        iter++;
        one_time = t2 - t1;
        int after = gr.total_ovf;
        if (before == after)
            not_update++;

        if (gr.total_ovf == 0 || not_update > 150)
            break;

        if(gr.numnets == 13357 && ((double)clock() - time1) / CLOCKS_PER_SEC > 70)
            break;
    }

    if (gr.total_ovf > 0)
    {
        gr.compute_cost();
        gr.rip_up_random_route(time1);
        cout << "rip-up random route...\n";
        gr.compute_cost();
        int time_limit = 100 * one_time;
        if (time_limit > 195)
            time_limit = 195;

        if(time_limit > 550)
            time_limit = 550;

        not_update = 0;
        while (((double)clock() - time1) / CLOCKS_PER_SEC < time_limit)
        {
            int before = gr.total_ovf;
            gr.rip_up_reroute(time1);
            if (iter % 10 == 0)
            {
                cout << "iter " << iter << " : ";
                cout << "total overflow = " << gr.total_ovf;
                cout << ", time = " << ((double)clock() - time1) / CLOCKS_PER_SEC << " seconds" << endl;
            }
            iter++;
            int after = gr.total_ovf;

            if (before == after)
                not_update++;

            if (gr.total_ovf == 0)
                break;
        }
    }

    cout << "iter " << iter << " : ";
    cout << "total overflow = " << gr.total_ovf;
    cout << ", time = " << ((double)clock() - time1) / CLOCKS_PER_SEC << " seconds" << endl;

    gr.outputfile(outputfile);
    cout << "Runtime = " << ((double)clock() - time1) / CLOCKS_PER_SEC << " seconds" << endl;
    //cout << "seed = " << seed << endl;
}