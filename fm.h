#ifndef _FM_H_
#define _FM_H_

#include "header.h"
using namespace std;

class Cell;
typedef shared_ptr<Cell> CellPtr;
class Cell{ 
	public:
		Cell();
		Cell(string instance_name,string libCell);
		
		std::string instance_name;
		std::string libCell;
		
		int area;	
		int gain;
		int bucket;
		int used;
		vector<string> all_net;
};
class Netss{
	public:
		Netss();
		Netss(int s,string net_name);
		string net_name;
		vector<CellPtr> net_cell;
		int cell_cnt;
};

class Graph{
	public:
		Graph();
		Graph(int die_area, int topdieutil,int bottomdieutil, int instancessize,int netssize,string topdietech,string bottomdietech);

		int total_cells;
		int total_nets;
		vector<CellPtr> vertices;

		int total_area;
		int top_area; 	 //total area of left bucket
		int bottom_area;  //total area of right bucket
		int biggest_area; //the biggest cell's area
		float r;

		int topdiemaxutil;
		int bottomdiemaxutil;
		string topdietech;
		string bottomdietech;

		vector<Netss> nets;
		
		int biggest_partial_sum;
		int partial_sum;
		pair<vector<string>,vector<string>> ans;

		int biggest_cell;  //biggest gain every round
		
		vector<pair<int,int>> add_to_bucket(CellPtr tmp,map<string,Tech> techs, vector<pair<int,int>> curr,DieRows topdierow,DieRows bottomdierow, int topdieutil, int bottomdieutil, int diearea);
		void init_gains();
		void find_max_gain();
		void net_change();
		void check_partial_sum();
};

FMANS fm(DieSize diesize, int topdieutil, int bottomdieutil, int instancessize, int netssize, string topdietech, string bottomdietech, map<string,string> instances, map<string,Net> nets, map<string,Tech> techs, DieRows topdierows, DieRows bottomdierows
);
Graph readfile(DieSize diesize, int topdieutil, int bottomdieutil, int instancessize, int netssize, string topdietech, string bottomdietech, map<string,string> instances, map<string,Net> nets, map<string,Tech> techs, DieRows topdierows, DieRows bottomdierows
);



#endif
