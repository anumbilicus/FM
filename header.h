#ifndef HEADER_H
#define HEADER_H


#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>

#include <fstream>
#include <sstream>

#include <cstdio>
#include <algorithm>
#include <limits>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <set>

//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>

using namespace std;

class libcell{
public:
    pair<int,int> size; //(size_x,size_y)
    map<string,pair<int,int>> pin_types;//{name,(x,y)}
};

class Tech{
public:
    map<string,libcell> cell_types;
};

class DieSize{
public:
    pair<int,int>lower;//(x,y)
    pair<int,int>upper;//(x,y)
};

class DieRows{
public:
    pair<int,int> start;//(x,y)
    int rowlength,rowheight;
    int repeatcount;
};
class Net{
public:
	map<string,string> connected_cell_pins ;
};

class FMANS{
public:
	pair<vector<string>,vector<string>> die;
	vector<string> terminal;
};

#endif

