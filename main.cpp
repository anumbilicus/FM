#include "header.h"
#include "fm.h"

using namespace std;

string topdietech,bottomdietech;
map<string,Tech> techs;//(tech_name/cell_types)
map<string,string> instances; //(instance_name/libCell)
map<string,Net> nets;//(net_name/)
DieRows topdierows,bottomdierows;
DieSize diesize;//(lower,upper)
pair<double,double> terminalsize;//(x,y)
int numtec,topdiemaxutil,bottomdiemaxutil,terminalspacing;

void read_data(char *a){
    string s;
    ifstream ifs(a, ios::in);
    if(!ifs.is_open()){
        cout << "Failed to open file.\n";
    }
    else{
        int libcell_count,libcell_x,libcell_y,pin_count,pin_x,pin_y;
        string tec_name,libcell_name,pin_name;
        
        ifs >> s/*NumTechnologies*/ >> numtec;
        for (int i=0;i<numtec;i++){
            Tech temp;
            ifs >> s/*Tech*/ >> tec_name >> libcell_count;
            for (int j=0;j<libcell_count;j++){
                ifs >> s/*LibCell*/ >> libcell_name >> libcell_x >> libcell_y >> pin_count;
                temp.cell_types[libcell_name].size.first = libcell_x;
                temp.cell_types[libcell_name].size.second = libcell_y;
                for(int k=0;k<pin_count;k++)  {  
                    ifs >> s/*Pin*/ >> pin_name >> pin_x >> pin_y;
                    temp.cell_types[libcell_name].pin_types[pin_name].first = pin_x;
                    temp.cell_types[libcell_name].pin_types[pin_name].second = pin_y;
                }
            }
            techs[tec_name] = temp;    
        }
        ifs >>  s/*DieSize*/ >> diesize.lower.first >> diesize.lower.second >> diesize.upper.first >> diesize.upper.second;
        ifs >> s/*TopDieMaxUtil*/ >> topdiemaxutil;
        ifs >> s/*BottomDieMaxUtil*/ >> bottomdiemaxutil;
        ifs >> s/*TopDieRows*/ >> topdierows.start.first >> topdierows.start.second >> topdierows.rowlength >> topdierows.rowheight >> topdierows.repeatcount;
        ifs >> s/*BottomDieRows*/ >> bottomdierows.start.first >> bottomdierows.start.second >> bottomdierows.rowlength >> bottomdierows.rowheight >> bottomdierows.repeatcount;
        ifs >> s/*TopDieTech*/ >> topdietech >> s/*BottomDieTech*/ >> bottomdietech;
        ifs >> s/*TerminalSize*/ >> terminalsize.first >> terminalsize.second;
        ifs >> s/*TerminalSpacing*/ >> terminalspacing;
        int numins;
        ifs >> s/*NumInstances*/ >> numins;
        for(int i=0;i<numins;i++){
            ifs >> s/*Inst*/ >> s >> instances[s];
        }
        int numnet;
        ifs >> s/*NumNets*/ >> numnet;
        for(int i=0;i<numnet;i++){
           string net_name;
           int pin_num;
           ifs >> s/*Net*/ >> net_name >> pin_num;
           for(int j=0;j<pin_num;j++){
               string cell_name,pin_name;
               ifs >> s/*Pin*/ >> s ;
               
               size_t idx = s.find("/");
               cell_name = s.substr(0,idx);
               pin_name = s.substr(idx+1);
               nets[net_name].connected_cell_pins[cell_name]=pin_name;
           }
        }

    }
    ifs.close();
}

void test(){
    cout << "NumTechnologies " << numtec << endl;

   	cout << endl;

    cout << "DieSize " << diesize.lower.first << " " << diesize.lower.second << " " << diesize.upper.first << " " << diesize.upper.second << endl;

    cout << endl;

    cout << "TopDieMaxUtil " << topdiemaxutil << endl;
    cout << "BottomDieMaxUtil " << bottomdiemaxutil << endl;
 
    cout << endl;

    cout << "TopDieRows " << topdierows.start.first << " " << topdierows.start.second << " " << topdierows.rowlength << " " <<topdierows.rowheight << " " << topdierows.repeatcount << endl;
    cout << "BottomDieRows " << bottomdierows.start.first << " " << bottomdierows.start.second << " " << bottomdierows.rowlength << " " <<bottomdierows.rowheight << " " << bottomdierows.repeatcount << endl;

    cout << endl;

    cout << "TopDieTech " << topdietech << endl;
    cout << "BottomDieTech " << bottomdietech << endl;

    cout << endl;

    cout << "TerminalSize " << terminalsize.first << " " << terminalsize.second << endl;
    cout << "TerminalSpacing " << terminalspacing << endl;

    cout << endl;
    //instance_name can use "for(auto iter=instances.begin();iter!=instances.end();++iter) cout << iter->first << endl;" find them all
    cout << "NumInstances " << instances.size() << endl;
    for(auto i:instances){
		cout << i.first << " " << i.second << endl; 
	}
	cout << endl;

    cout << "NumNets " << nets.size() << endl;
}

///////////////////////
//////placement///////
//////////////////////
class Location{
public:
    pair<int,int> location; //(loation_x,location_y)
};
int topdieplacement = 0;
int bottomdieplacement = 0;
int numterminals = 0;
map<string,Location> top_out,bottom_out,terminal_out; //instance_name,(locationx,locationy)
map<string,string> Die_pos;


void naive_placement(){
	////current die(top or bottom) start from top
	string curr_die = "Top";
	////current tech
	string curr_tech =  topdietech ;
	////current die rows
	DieRows curr_dierows = topdierows;
	////current placement of the current die
	pair<int,int> curr_placement(curr_dierows.start.first,curr_dierows.start.second); //initial from (start_x,start_y)
	//// current used area
	int sum_area = 0;
	
	////die area
	unsigned int die_area = (diesize.upper.first - diesize.lower.first) * (diesize.upper.second - diesize.lower.second);
	////for every instance in netlist	
	for(const auto& inst: instances){
		//cout << "instance: "<<inst.first<<" library cell types: "<<inst.second<< endl;
		////use curr_tech to find length and width and area(library)
		int x = techs[curr_tech].cell_types[inst.second].size.first;
		int y = techs[curr_tech].cell_types[inst.second].size.second;
		////calculate utilization(with this instance)
		////smaller -> in orginal tier ; bigger -> place on another tier 
		sum_area += x*y;
		//cout <<"sum area: "<<sum_area<<endl;
		if((curr_die == "Top") && (((float)sum_area)/((float)die_area))*100 > topdiemaxutil){
			curr_die = "Bottom";
			curr_tech = bottomdietech;
			curr_dierows = bottomdierows;
			x = techs[curr_tech].cell_types[inst.second].size.first;
			y = techs[curr_tech].cell_types[inst.second].size.second;
			sum_area =x*y;
			curr_placement.first = 0;
			curr_placement.second = 0;
		}
		
		//place on the correct location/row
		if(curr_placement.first + x > curr_dierows.rowlength){ //change to next row
			curr_placement.first = x;
			curr_placement.second += curr_dierows.rowheight;
			if(curr_die == "Top"){
				topdieplacement += 1;
				top_out[inst.first].location.first = 0;
				top_out[inst.first].location.second = curr_placement.second; 
				Die_pos[inst.first] = "top" ;
			}
			else{
				bottomdieplacement += 1;
				bottom_out[inst.first].location.first = 0;
				bottom_out[inst.first].location.second = curr_placement.second;
				Die_pos[inst.first] = "bot";
			}
		}
		else{
			curr_placement.first += x;
			//current_placement.second don't change
			if(curr_die == "Top"){
				topdieplacement += 1;
				top_out[inst.first].location.first = curr_placement.first-x;
				top_out[inst.first].location.second = curr_placement.second; 
				Die_pos[inst.first] = "top" ;
			}
			else{
				bottomdieplacement += 1;
				bottom_out[inst.first].location.first = curr_placement.first-x;
				bottom_out[inst.first].location.second = curr_placement.second;
				Die_pos[inst.first] = "bot";
			}
		}
	}
}

void print_out(){
	cout<<"\n\n"<<endl;
	cout << "TopDiePlacement "<<topdieplacement<<endl;
	for(const auto& inst: top_out)
		cout<<"Inst "<<inst.first <<" "<<inst.second.location.first <<" " <<inst.second.location.second <<endl;
	
	cout << "BottomDiePlacement "<<bottomdieplacement<<endl;
	for(const auto& inst: bottom_out)
		cout<<"Inst "<<inst.first <<" "<<inst.second.location.first <<" " <<inst.second.location.second <<endl;
}


map<pair<string,string>,pair<int,int>> Net_pin_position;


void set_pin(){
	for(auto i: instances){
			if(Die_pos[i.first] == "top"){
				for(auto j:techs[topdietech].cell_types[i.second].pin_types){
					Net_pin_position[make_pair(i.first,j.first)].first = top_out[i.first].location.first + j.second.first;
					Net_pin_position[make_pair(i.first,j.first)].second = top_out[i.first].location.second + j.second.second;
					//cout << i.first << " " << j.first << " " << j.second.first << " " << j.second.second <<  endl;
				}
			}
			else if(Die_pos[i.first] == "bot"){
				for(auto j:techs[bottomdietech].cell_types[i.second].pin_types){
					Net_pin_position[make_pair(i.first,j.first)].first = bottom_out[i.first].location.first + j.second.first;
					Net_pin_position[make_pair(i.first,j.first)].second = bottom_out[i.first].location.second + j.second.second;
					//cout << i.first << " " << j.first << " " << j.second.first << " " << j.second.second <<  endl;
				}

			}
	}
}

vector<string> term_nets;
map<string,pair<double,double>> terminal;
bool top_exist,bot_exist = false;

void terminal_compute(){
	for(auto i : nets){
		for(auto j:i.second.connected_cell_pins){
			if(Die_pos[j.first] == "top"){
				top_exist = true;
			}
			else if (Die_pos[j.first] == "bot"){
				bot_exist = true;
			}
		}

		if(top_exist && bot_exist){
			term_nets.push_back(i.first);		
		}
		top_exist = false;
		bot_exist = false;
	}
	
}
void terminal_generate(){
	int bound_x = terminalspacing + ceil(terminalsize.first/2);
	int bound_y = terminalspacing + ceil(terminalsize.second/2);

	
	int change_row = 0;
	int term_count = 0;
	for(auto i:term_nets){
		terminal[i] = make_pair(diesize.lower.first + bound_x + 2 * term_count * terminalsize.first,
							diesize.lower.second + bound_y + 2 * change_row * terminalsize.second); // from lower left create terminal
		if(terminal[i].first + bound_x + 2 * terminalsize.first > diesize.upper.first){ // if row has been over spacing
			change_row ++ ; // change row
			term_count = 0;
		}
		else{
			term_count ++;
		}
	}
	
}




void output_data(char * out){
	ofstream outf(out,ios::out);
	outf << "TopDiePlacement " << topdieplacement << endl;
	for(auto i:top_out){
		outf << "Inst " << i.first << " " << i.second.location.first << " " << i.second.location.second << endl;
	}
	outf << "BottomDiePlacement " << bottomdieplacement << endl;
	for(auto i:bottom_out){
		outf << "Inst " << i.first << " " << i.second.location.first << " " << i.second.location.second << endl;
	}
	outf << "NumTerminals " << terminal.size() << endl;
	for(auto i:terminal){
		outf << "Terminal " << i.first << " " << i.second.first << " " << i.second.second << endl;
	}

}

int main(int argc,char *argv[]){
    read_data(argv[1]);
    //test();
	
	FMANS fm_ans;
	fm_ans = fm(diesize,topdiemaxutil, bottomdiemaxutil, instances.size(), nets.size(), topdietech,  bottomdietech, instances, nets, techs, topdierows, bottomdierows);

	/////////////test/////////////
	cout<<"cells on top die"<<endl;
	for(int i=0;i<fm_ans.die.first.size();i++)
		cout<<fm_ans.die.first[i]<<" ";
	
	cout<<"\ncells on bottom die"<<endl;
	for(int i=0;i<fm_ans.die.second.size();i++)
		cout<<fm_ans.die.second[i]<<" ";

	cout<<"\nterminal:"<<endl;
	for(int i=0;i<fm_ans.terminal.size();i++)
		cout<<fm_ans.terminal[i]<<" ";

	//naive_placement();
	//print_out();
	//set_pin();
	//terminal_compute();
	//terminal_generate();
	//output_data(argv[2]);

    return 0;
}
