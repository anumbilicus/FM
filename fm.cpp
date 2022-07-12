#include "fm.h"
#include "header.h"

using namespace std;

Cell::Cell(){}
Cell::Cell(string instance_name,string libCell){
	this->instance_name = instance_name;
	this->libCell = libCell;
	this->used = 0;
}
Netss::Netss(){}
Netss::Netss(int s,string net_name){
	this -> cell_cnt = s;
	this -> net_name = net_name;
}

Graph::Graph(){}
Graph::Graph(int die_area, int topdiemaxutil, int bottomdiemaxutil, int instancessize, int netssize, string topdietech, string bottomdietech){
	this -> total_area = die_area;
	this -> top_area = 0;    // =left_area
	this -> bottom_area = 0; // =right_area
	
	this -> biggest_area = 0;
	this -> biggest_cell = 0;
	this -> partial_sum = 0;
	this -> biggest_partial_sum = -1000000;

	this -> topdiemaxutil = topdiemaxutil;
 	this -> bottomdiemaxutil = bottomdiemaxutil;
	this -> total_cells = instancessize;
	this -> total_nets = netssize;
	this -> topdietech = topdietech;
	this -> bottomdietech = bottomdietech;
}

//////////////////////////////
//put cell into bucket
////////////////////////////////
vector<pair<int,int>> Graph::add_to_bucket(CellPtr tmp,map<string,Tech> techs, vector<pair<int,int>> curr, DieRows topdierow, DieRows bottomdierow, int topdieutil, int bottomdieutil, int diearea){
	int currdie;
	string currdietech;
	DieRows currdierow;
	
	//check util
	int cell_area = techs[topdietech].cell_types[tmp->libCell].size.first * techs[topdietech].cell_types[tmp->libCell].size.second;
	int tmp_area = this->top_area + cell_area;
	if( ((float)tmp_area/(float)this->total_area)*100 > topdiemaxutil){
		//cell's length in bottom die
		currdie = 1;
		currdietech = bottomdietech;
		currdierow  = bottomdierow;
	}
	else{
		//cell's length in top die
		currdie = 0;
		currdietech = topdietech;
		currdierow  = topdierow;
	}

	int cell_length = techs[currdietech].cell_types[tmp->libCell].size.first;
	cell_area = techs[currdietech].cell_types[tmp->libCell].size.first * techs[currdietech].cell_types[tmp->libCell].size.second;
	
	if((curr[currdie].first + cell_length) <= currdierow.rowlength){
		curr[currdie].first += cell_length;
	}
	else if((curr[currdie].first + cell_length) > currdierow.rowlength){
		if((curr[currdie].second + 1) <= currdierow.repeatcount){
			curr[currdie].first = cell_length;	//change row
			curr[currdie].second += 1;
		}
		else cout<<"errorrrrrrrrr"<<endl;
	}
	
	if(currdietech == topdietech)
		this->top_area += cell_area;
	else 
		this->bottom_area += cell_area;

	tmp->area = cell_area;
	tmp->bucket = currdie;
	this->vertices.push_back(tmp);

	return curr;
}

/////////////////////////////////
//initialize cell's gains
/////////////////////////////////
void Graph::init_gains(){

	//traverse all cells in every nets
	for(int i=0;i<this->total_nets;i++){
		for(int j=0;j<nets[i].cell_cnt;j++){
			
			//find cell j in which bucket
			int cell_bucket = nets[i].net_cell[j]->bucket;

			int from_cell = 0;
			int to_cell = 0;

			//check other cell in the same net
			for(int k=0;k<nets[i].cell_cnt;k++){
				if(nets[i].net_cell[k]->bucket != cell_bucket)
					to_cell+=1;
				else from_cell+=1;
			}

			if(to_cell == 0)
				nets[i].net_cell[j]->gain -= 1;
			if(from_cell == 1)
				nets[i].net_cell[j]->gain += 1;
		}	
	}

	////////test
	/*cout<<"******************test initial gain**********************"<<endl;
	for(int i=0;i<this->total_cells;i++)
		cout<<"cell name: "<<vertices[i]->instance_name<<" cell gain: "<<vertices[i]->gain<<endl;
	*/
}

////////////////////////////////
//to pick the biggest gain 
////////////////////////////////
void Graph::find_max_gain(){

	int biggest_gain = -1000000000;
	int biggest_cell;
	int tmp_area;
	
	for(int i=0;i< this->total_cells;i++){
		
		//find cell that isn't used
		if(vertices[i]->used == 0 && vertices[i]->gain >= biggest_gain){
			
			//check if meet die's maxutil
			if(vertices[i]->bucket == 0){ //cell on top die
				tmp_area = (float)(this->bottom_area+vertices[i]->area) / (float)(this->total_area); 
				if(tmp_area*100 <= this->bottomdiemaxutil){
					//tie
					if( (vertices[i]->gain == biggest_gain) && (vertices[i]->area < vertices[biggest_cell]->area) )
						biggest_cell = i;
					//not tie
					else if(vertices[i]->gain > biggest_gain){
						biggest_cell = i;
						biggest_gain = vertices[i]->gain;
					}
				}
			}
			else{ //cell on bottom die
				tmp_area = (float)(this->top_area+vertices[i]->area) / (float)(this->total_area); 
				if(tmp_area*100 <= this->topdiemaxutil){
					//tie
					if( (vertices[i]->gain == biggest_gain) && (vertices[i]->area < vertices[biggest_cell]->area) )
						biggest_cell = i;
					//not tie
					else if(vertices[i]->gain > biggest_gain){
						biggest_cell = i;
						biggest_gain = vertices[i]->gain;
					}
				}
			}
		}
	}

	this->biggest_cell = biggest_cell;
	//cout<<"biggest cell:"<<vertices[biggest_cell]->instance_name<<" biggest gain:"<<biggest_gain<<endl;
}

void Graph::net_change(){
	
	//partial sum
	this->partial_sum += vertices[this->biggest_cell]->gain;
	//cout<<"partial sum:"<<this->partial_sum<<endl;

	int curr_bucket = vertices[this->biggest_cell]->bucket;
	int curr_net;
	int from = 0;
	int to = 0;

	//////////////
	//before move
	/////////////
	
	//traverse every net
	for(int i=0; i<vertices[this->biggest_cell]->all_net.size(); i++){
		curr_net = stoi(vertices[this->biggest_cell]->all_net[i].substr(1))-1;
		//cout<<"current net:"<<vertices[this->biggest_cell]->all_net[i]<<endl;

		for(int j=0;j<nets[curr_net].net_cell.size();j++){
			if(nets[curr_net].net_cell[j]->used == 0){
				if(nets[curr_net].net_cell[j]->bucket != curr_bucket){
					to += 1;
					//cout<<"to++"<<endl;
				}
			}
		}

		if(to == 0){
			for(int k=0;k<nets[curr_net].net_cell.size();k++){
				if(nets[curr_net].net_cell[k]->bucket == curr_bucket)
					nets[curr_net].net_cell[k]->gain += 1;
			}
		}
		else if(to == 1){
			for(int k=0;k<nets[curr_net].net_cell.size();k++){
				if(nets[curr_net].net_cell[k]->bucket != curr_bucket)
					nets[curr_net].net_cell[k]->gain -= 1;
			}
		}

		to = 0;
	}

	////////////////
	//move
	////////////////
	
	vertices[this->biggest_cell]->used = 1;
	if(curr_bucket == 0){
		vertices[this->biggest_cell]->bucket = 1;
		this->top_area -= vertices[this->biggest_cell]->area;
		this->bottom_area += vertices[this->biggest_cell]->area;
	}
	else{
		vertices[this->biggest_cell]->bucket = 0;
		this->top_area += vertices[this->biggest_cell]->area;
		this->bottom_area -= vertices[this->biggest_cell]->area;
	}

	////////////////
	//after move
	////////////////
	
	for(int i=0; i<vertices[this->biggest_cell]->all_net.size(); i++){
		curr_net = stoi(vertices[this->biggest_cell]->all_net[i].substr(1))-1;
		//cout<<"current net:"<<vertices[this->biggest_cell]->all_net[i]<<endl;

		for(int j=0;j<nets[curr_net].net_cell.size();j++){
			if(nets[curr_net].net_cell[j]->used == 0){
				if(nets[curr_net].net_cell[j]->bucket == curr_bucket){
					from += 1;
					//cout<<"from++"<<endl;
				}
			}
		}
		if(from == 0){
			for(int k=0;k<nets[curr_net].net_cell.size();k++){
				if(nets[curr_net].net_cell[k]->bucket != curr_bucket)
					nets[curr_net].net_cell[k]->gain -= 1;
			}
		}
		else if(from == 1){
			for(int k=0;k<nets[curr_net].net_cell.size();k++){
				if(nets[curr_net].net_cell[k]->bucket == curr_bucket)
					nets[curr_net].net_cell[k]->gain += 1;
			}
		}
		from = 0;
	}
}

void Graph::check_partial_sum(){
	if(this->partial_sum > this->biggest_partial_sum){
		this->biggest_partial_sum = this->partial_sum;

		this->ans.first.clear();
		this->ans.second.clear();
		for(int i=0;i<vertices.size();i++){
			if(vertices[i]->bucket == 0)
				ans.first.push_back(vertices[i]->instance_name);
			else
				ans.second.push_back(vertices[i]->instance_name);
		}
	}
}



//////////////////////////////
//change data into fitted type 
//////////////////////////////
Graph readfile(DieSize diesize, int topdieutil, int bottomdieutil, int instancessize, int netssize, string topdietech, string bottomdietech, map<string,string> instances, map<string,Net> nets, map<string,Tech> techs, DieRows topdierows, DieRows bottomdierows){

	////////declaration graph//////////////
	int diearea = diesize.upper.first * diesize.upper.second;
	Graph graph(diearea, topdieutil, bottomdieutil, instancessize, netssize, topdietech, bottomdietech);
	
	//cout<<"************create graph**************"<<endl;
	//cout<<"dies area= "<<diearea<<endl;

	///////////cell//////////////
	//every round of current row info
	vector<pair<int,int>> curr;       //(curr_rowlength, curr_repeatcount)
	curr.push_back(make_pair(0,0));   //top
	curr.push_back(make_pair(0,0));	  //bottom

	//cout<<"***************create cell***************"<<endl;
	for(const auto& inst:instances){
				            //inst_name   //libcell
		CellPtr tmp(new Cell(inst.first, inst.second));
		curr = graph.add_to_bucket(tmp,techs, curr, topdierows, bottomdierows,topdieutil, bottomdieutil,diearea);
		
		//cout<<"cell name:"<<tmp->instance_name<<" cell lib:"<<tmp->libCell<<endl;
		//cout<<"top rest area:"<<curr[0].first<<" "<<curr[0].second<<" bottom rest area:"<<curr[1].first<<" "<<curr[1].second<<endl;
	}

	//////test//////////
	/*
	cout<<"***********test****************"<<endl;
	for(int i=0;i<graph.vertices.size();i++)
		cout<<graph.vertices[i]->instance_name<<" "<<graph.vertices[i]->area<<" "<<graph.vertices[i]->bucket<<endl;
	*/
	
	//////////net//////
	//cout<<"**************nets************"<<endl;
	for(const auto& n:nets){
		//n.first = net name ; n.second = net(cell name/pin name) 
		Netss nn(nets[n.first].connected_cell_pins.size(),n.first);
		
		//cout<<"Net "<<n.first<<endl;
		
		for(const auto& c:nets[n.first].connected_cell_pins){
			
			//cout<<"inst:"<<c.first<<" pin:"<<c.second<<endl;
			
			graph.vertices.at(stoi(c.first.substr(1))-1)->all_net.push_back(n.first);
			nn.net_cell.push_back(graph.vertices.at(stoi(c.first.substr(1))-1));
		}
		graph.nets.push_back(nn);
	}
	
	/*cout<<"//////test net///////"<<endl;
	for(int i=0;i<graph.vertices.size();i++){
		cout<<"\ncell "<<i+1<<endl;
		for(int j=0;j<graph.vertices[i]->all_net.size();j++)
			cout<<graph.vertices[i]->all_net[j]<<" ";	
	}
	for(int i=0;i<graph.nets.size();i++){
		cout<<"\nNet "<<i+1<<endl;
		for(int j=0;j<graph.nets[i].cell_cnt;j++)
		cout<<graph.nets[i].net_cell[j]->instance_name<<" ";
	}*/

	return graph;
}


//DieSize diesize, int topdieutil, int bottomdieutil, int instances.size(), int nets.size(), string topdietech, string bottomdietech, map<string,string> instances, map<string,Tech> techs, DieRows topdierows, DieRows bottomdierows
FMANS fm(DieSize diesize, int topdieutil, int bottomdieutil, int instancessize, int netssize, string topdietech, string bottomdietech, map<string,string> instances, map<string,Net> nets, map<string,Tech> techs, DieRows topdierows, DieRows bottomdierows){
	
	Graph graph;
	graph = readfile(diesize, topdieutil, bottomdieutil, instancessize, netssize, topdietech, bottomdietech, instances,nets,techs, topdierows, bottomdierows);
	
	//cout<<"\ntop die area:"<<graph.top_area<<" bottom die area:"<<graph.bottom_area<<endl;

	//cout<<"*********initialize gain*********"<<endl;
	graph.init_gains();
	
	for(int i=0;i<graph.vertices.size();i++){
		//cout<<"*********find max gain*********"<<endl;
		graph.find_max_gain();

		//cout<<"*********net change*************"<<endl;
		graph.net_change();
		
		//cout<<"********check partial sum******"<<endl;
		if(i!=0)
			graph.check_partial_sum();
	}

	//cout<<"\n*************answer******************"<<endl;

	FMANS fm_ans;

	//cout<<"cell on top die"<<endl;
	for(int i=0;i<graph.ans.first.size();i++){
		//cout<<graph.ans.first[i]<<" ";

		fm_ans.die.first.push_back(graph.ans.first[i]); 
		graph.vertices[stoi(graph.ans.first[i].substr(1))-1]->bucket = 0;
	}
	//cout<<"\ncell on bottom die"<<endl;
	for(int i=0;i<graph.ans.second.size();i++){
		//cout<<graph.ans.second[i]<<" ";
		
		fm_ans.die.second.push_back(graph.ans.second[i]); 
		graph.vertices[stoi(graph.ans.second[i].substr(1))-1]->bucket = 1;
	}

	vector<string> terminal;
	//cout<<"\nterminal"<<endl;
	for(int i=0;i<graph.nets.size();i++){
		int t=0;
		int b=0;
		for(int j=0;j<graph.nets[i].cell_cnt;j++){
			if(graph.nets[i].net_cell[j]->bucket == 0) t+=1;
			else b+=1;
		}
		if(t!=0 && b!=0){
			//cout<<graph.nets[i].net_name<<" ";
			fm_ans.terminal.push_back(graph.nets[i].net_name);
		}
	}

	return fm_ans;
}





