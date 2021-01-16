/**********************************
Author: Michael Douglas - 0403066
Author: Ruan Felipe - 0397048
UFC - Campus Quixadá
**********************************/

#include <bits/stdc++.h>

using namespace std;

// Prioridade
typedef long long   ll;
typedef pair<int,int> pii;
typedef vector<int> vi;
typedef vector<ll> vll;
typedef vector<pii> vpi;

#define F first
#define S second
#define PB push_back
#define MP make_pair
#define REP(i,a,b) for(int i = a; i < (b); i++)
#define INF 0x3f3f3f3f
#define INFLL 0x3f3f3f3f3f3f3f3f
#define all(x) x.begin(),x.end()
#define MOD 1000000007
#define endl '\n'
#define mdc(a, b) (__gcd((a), (b)))
#define mmc(a, b) (((a)*(b))/__gcd((a), (b)))

// fim da Prioridade

#define L1_OFFSET 							(0xFFF00000)
#define L2_OFFSET_in_coarse_page			(0x000FF000)
#define L2_OFFSET_in_fine_page				(0x000FFC00)
#define Section_Entry_BaseAddr        		(0xFFF00000)
#define	Coarse_PageTable_BaseAddr     		(0xFFFFFC00)
#define Fine_PageTable_BaseAddr        		(0xFFFFF000)

#define offset_real_page_section            (0x000FFFFF)
#define offset_large_page                   (0x0000FFFF)
#define offset_small_page                   (0x00000FFF)
#define offset_tiny_page                    (0x000003FF)

#define Large_Page_BaseAddr                 (0xFFFF0000)
#define Small_Page_BaseAddr                 (0xFFFFF000)
#define Tiny_Page_BaseAddr                  (0xFFFFFC00)

#define COARSE                              (0x1)
#define SECTION  							(0x2)
#define FINE                                (0x3)

bool fill(void);
void change_cache(int address_physical);
bool FindInMMU(int address_virtual);
int FindInL2(int baseaddr, int OFFSET, int address_virtual, int type);
bool ReadAddress();
void printCache();


struct memory_cache{
	int cache_tag;
	int hit;
	int miss;
	memory_cache(){
		cache_tag = -1;
		hit = 0;
		miss = 0;
	}
};


vector<memory_cache> PhysicalCache;
vector<int> Memory_L1;
vector<int> Memory_L2;
//static int cot = 0;
bool fill(){
	//return true if both files has been read, else return false.
	ifstream L1("L1.txt");
	if(L1.is_open()){
		stringstream ss;
		unsigned int aux;
		string line;
		while(getline(L1,line)){
			ss << line;
			aux = 0;
			ss >> hex >> aux;
			ss.clear();
			Memory_L1.PB(aux);
		}
	}else{
		return false;
	}
	L1.close();
	ifstream L2("L2.txt");
	if(L2.is_open()){
		stringstream ss;
		unsigned int aux;
		string line;
		while(getline(L2,line)){
			ss << line;
			aux = 0;
			ss >> hex >> aux;
			ss.clear();
			Memory_L2.PB(aux);
		}
	}else{
		return false;
	}
	L2.close();
	return true;
}

void change_cache(int address_physical){
	int cache_tag = (address_physical & 0xFFFFFC00) >> 10; // offset tag
	int setindex  = (address_physical & 0x000003FC) >> 2; // offset setindex 
	if(PhysicalCache[setindex].cache_tag == cache_tag){
		PhysicalCache[setindex].hit++;
	}else{
		PhysicalCache[setindex].cache_tag = cache_tag;
		PhysicalCache[setindex].miss++;
	}
	//cot++;
}

bool FindInMMU(int address_virtual){
	int address_L1 = (address_virtual & L1_OFFSET) >> 20;
	int OFFSET_L2;
		

	int PTE = Memory_L1[address_L1]; // page table entry
	int last_bits = PTE & (3);
	if(last_bits == 1){
		//Coarse Page Table
		int base_addr = (PTE & Coarse_PageTable_BaseAddr);
		OFFSET_L2 = (address_virtual & L2_OFFSET_in_coarse_page);
		OFFSET_L2 = OFFSET_L2 >> 12;
		FindInL2(base_addr, OFFSET_L2, address_virtual, last_bits);		
		return true;
	}else if(last_bits == 2){
		//Section Entry, in this case don't need find in L2 
		int base_addr = (PTE & Section_Entry_BaseAddr);
		int offset = (address_virtual & offset_real_page_section);
		change_cache(base_addr + offset);
		return true;
	}else if(last_bits == 3){
		//Fine Page Table
		int base_addr = (PTE & Fine_PageTable_BaseAddr);
		OFFSET_L2 = (address_virtual & L2_OFFSET_in_fine_page);
		OFFSET_L2 = OFFSET_L2 >> 10;
		FindInL2(base_addr, OFFSET_L2, address_virtual, last_bits);
		return true;
	}else{
		//Fault
		cout << "Error: Page Fault" << endl;
		return 0;
	}
}

int FindInL2(int baseaddr, int OFFSET, int address_virtual, int type){
		int PTE_L2 = Memory_L2[OFFSET];
		int last_bits = PTE_L2 & (0x3);
		int OFFSET_Address_Real;
		if(last_bits == 1){
			//Large Page Table
			int base_address_physics = (PTE_L2 & Large_Page_BaseAddr);
			OFFSET_Address_Real = (address_virtual & offset_large_page);
			change_cache(base_address_physics + OFFSET_Address_Real);
			return 1; 
		}else if(last_bits == 2){
			//Small Page Table
			int base_address_physics = (PTE_L2 & Small_Page_BaseAddr);
			OFFSET_Address_Real = (address_virtual & offset_small_page);
			change_cache(base_address_physics + OFFSET_Address_Real);
			return 1;	
		}else if(last_bits == 3){
			//Tiny Page Table
			if(type == COARSE){
				return 0;
			}
			int base_address_physics = (PTE_L2 & Tiny_Page_BaseAddr);
			OFFSET_Address_Real = (address_virtual & offset_tiny_page);
			change_cache(base_address_physics + OFFSET_Address_Real);
			return 1;
		}else{
			//Fault
			cout << "Error: Page Fault" << endl;
			return 0;
		}				
			
}
bool ReadAddress(){
	//return true if the file has been read, else return false.
	ifstream address("ADDRESS.txt");
	if(address.is_open()){
		stringstream ss;
		unsigned int aux;
		string line;
		while(getline(address, line)){
			ss << line;
			aux = 0;
			ss >> hex >> aux;
			ss.clear();
			FindInMMU(aux);
		}
	}else{
		return false;
	}
	return true;
}

void printCache(){
		
		
		for(unsigned int i = 0; i < PhysicalCache.size(); ++i){
		  if(PhysicalCache[i].cache_tag != -1){
			cout << hex << i << ": ";	
			cout << "[0x" << uppercase << hex << setw(8) << setfill('0') << PhysicalCache[i].cache_tag << "]";
			cout << "[0x" << hex << setw(8) << setfill('0') << (PhysicalCache[i].cache_tag<<10)+(i<<2)+0x0 <<  "-0x" << hex << setw(8) << setfill('0') << (PhysicalCache[i].cache_tag<<10)+(i<<2)+0x3 << "]";
			cout << " H(" << dec << PhysicalCache[i].hit << ")";
			cout << " M(" << PhysicalCache[i].miss << ")" << endl;
		  
		  }	
		}
}	

int main() {
	//initializing the memory cache;
		PhysicalCache.resize(256,memory_cache());
	//end

	if(fill() == false){
		cout << "L1 or L2 file not found." << endl;
		return 0; 
	}
	if(ReadAddress() == false){
		cout << "Address file not found" << endl;
		return 0;
	}
	printCache();
	
	return 0;
}
