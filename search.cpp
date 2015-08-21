/*
/*
 * search.cpp
 *
 *  Created on: Jun 8, 2015
 *      Author: David
 *
 *
 *  REQUIRES BOOST
 *  TO COMPILE: g++ -g -std=c++0x search.cpp -L/usr/local/lib -lboost_random -lboost_system
 */



extern "C" {
#include <sys/time.h>   /* gettimeofday() */
#include <libgen.h>       /* basename() */
#include <unistd.h>
#include <errno.h>
}
#include <limits>
#include <fstream>
#include <iostream>
#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <map>


#include <cstdio>
#include <cstdlib>
#include <cstdint>      /* uint*_t  */
#include <cstddef>      /* offsetof */
#include <cstring>      /* memcpy() */
#include <ctime>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <algorithm>

#include <boost/thread/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


#define START "starting string";									//These strings are important. Used as identifiers so we know when a valid pattern has appeared.
#define END "ending string";										//They must be unique and separate from all patterns.


	typedef struct pt_struct {										//Pattern_Structs are basically four tuples of strings
		std::string whole_pattern;									//The whole_pattern represents the overall pattern, using example A*B, it would be A*B
		std::string prev_pattern;									//The prev_pattern represents the previous pattern and is used only for examples like A*B. B would have a prev of A, A would have a prev of START
		std::string next_pattern;									//The next_pattern is the counterpart to prev_pattern. Stores the next supposed pattern. A would have a next of B, B would have a next of END.
		std::string this_pattern;									//This pattern is the actual pattern itself. We can check: Does A's next = B and B's prev =A? We need both because if we only had one, we could get
																	//the example B, see it has an end and return the pattern B, when A has not occurred. We must check if B has a prev too.
		int index;													//Used in the obscure case of A*B, A= abcdef B = cdefghi, accepting abcdefghi
	} Pattern_Struct;

	typedef std::vector<Pattern_Struct> Pattern_List;				//Vector of pointers to Pattern_Structs so they can be used for storing a dynamic blah.
	static unsigned int HashCode(const char* str);					//All of the static function declarations.
	static unsigned int HashCode(const char* str, int len);

	static unsigned int HashCode(const char* str) {
	    unsigned int hash = 0;
	    while (*str != '\0')
	        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	    return (hash & 0x7FFFFFFF);
	}

	static unsigned int HashCode(const char* str, int len){
	    unsigned int hash = 0;
	    while (*str != '\0' && len > 0) {
	        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	        --len;
	    }
	    return (hash & 0x7FFFFFFF);
	}

	class Hash_Table{												//Structure for the Hash Table.
	public:															//The goal of this Hash Table is to store keys of whole_patterns and values of Pattern_Structs.
																	//This Hash Table is unique in that when a value is stored, we first attempt to find the other patterns
																	//it is part of a group of. For example, given target pattern A*B, when we come across B, we would search
																	//for A in the Hash_Table. This is made possible due to the Pattern_Struct's prev and next values, so we know what we're looking for.
																	//Once we find the previous value, we can update it. At the end, we will provide a function that cycles through the hash table and only returns complete pairs.
		Hash_Table(int);
		~Hash_Table();

		void insert(Pattern_Struct pat);							//Insertion. Special in a way that it can merely alter as described above. Deletion is not needed for this Hash Table
		std::vector<std::string> compile();							//Compile. This is the function described above. It compiles all the correct matches and reports them as a vector.

	private:
		int buckets;
		int elements;
		Pattern_List *table;

		void resize();

	};

	Hash_Table::Hash_Table(int bucket){								//Constructor. Initializes with this many buckets
	    buckets = bucket;
	    elements = 0;
	    table = new Pattern_List[bucket];
	}

	Hash_Table::~Hash_Table(){										//Destructor
		delete[] table;
	}

	void Hash_Table::insert(Pattern_Struct pat){
		if(buckets < elements){
			elements = 0;
			resize();
		}
		elements++;
		int hash = HashCode(pat.whole_pattern.c_str()) % buckets;
		Pattern_List &elem = table[hash];
		Pattern_List::iterator iter = elem.begin();
		while(iter != elem.end()){										
			if(iter->whole_pattern == pat.whole_pattern &&			//Does not handle the case of A*A*A*B, given A*B, will match.
					iter->next_pattern == pat.this_pattern &&
					iter->this_pattern == pat.prev_pattern &&
					(iter->index + iter->this_pattern.length()) <= pat.index){
				iter->this_pattern = pat.this_pattern;
				iter->next_pattern = pat.next_pattern;
				return;												//Exits the loop so it doesn't get added below
			}
			iter++;
		}
		table[hash].push_back(pat);
	}

	void Hash_Table::resize(){										//Standard resizing of elements. Load factor is 1.0
		Pattern_List *old = table;									//We try to keep a variable referring to the old list.
		table = new Pattern_List[buckets*2];						//And then we can extract out old elements and put it in the new list.
		for(int i = 0; i < buckets; i++){							//Might not work as intended though.
			Pattern_List &elem = old[i];
			Pattern_List::iterator iter = elem.begin();
			while(iter != elem.end()){
				insert(*iter);
				iter++;
			}
		}
		buckets *= 2;
		delete[] old;												//I think this is correct deletion.
	}

	std::vector<std::string> Hash_Table::compile(){					//Goes through the hash table, and tries to find valid prev and next patterns.
		std::vector<std::string> lst;
		for(int i = 0; i < buckets; i++){
			Pattern_List &elem = table[i];
			Pattern_List::iterator iter = elem.begin();
			while(iter != elem.end()){
				if(iter->prev_pattern == "START" &&					//Once one is found, we simply return the whole_pattern.
					iter->next_pattern == "END"){					//This list is all the patterns that occur with the target string
					lst.push_back(iter->whole_pattern);				//I guess I can do some check here but performance may suffer.
				}
				iter++;
			}
		}
		return lst;
	}

	class Trie{
	public:
		Trie();
		~Trie();
		void insert(Pattern_Struct pat);
		void insert(const char *target, Pattern_Struct pat);
		void traverse(Hash_Table *ht, const char *target, int index);

	private:
		Trie *array[128];											//Pointer to 128 other Tries
		Pattern_List pattern;										//Pattern_Struct if this Trie node is a match. This is a vector in case there are multiple patterns.
		bool match;													//Boolean used to determine if this node is a match
	};

	Trie::Trie(){
		match = false;
		//Everything should already be zero initialized.
	}

	Trie::~Trie(){
		//VOID because I'm ignoring it.
	}

	void Trie::insert(Pattern_Struct pat){
		insert(pat.this_pattern.c_str(), pat);
	}

	void Trie::insert(const char *target, Pattern_Struct pat){
		if(*target == '\0'){											//End of the line.
			match = true;
			pattern.push_back(pat);
			return;
		}
		else if(array[*target] == nullptr)								//We want to create a new Trie if one doesn't yet exist down there.
			array[*target] = new Trie();

		array[*target]->insert(target+1, pat);
	}

	void Trie::traverse(Hash_Table *ht, const char *target, int index){	//Want to traverse through the entire array
		if(match){
			Pattern_List::iterator iter = pattern.begin();
			while(pattern.end() != iter){
				iter->index = index;
				ht->insert(*iter);
				iter++;
			}
		}
		if(*target == '\0' || array[*target] == nullptr)				//If we reach the end of the target, simply return. Otherwise we can continue
			return;
		array[*target]->traverse(ht, target+1, index);
	}

	class WuManber{
	private:
		int table_size;
		int mBlock;
		int mMin;
		std::vector<int32_t> mShiftTable;
		Trie *trie;

		std::vector<std::string> search(const char *target, int length);

	public:
		WuManber();
		~WuManber();
		std::vector<std::string> search(std::string target);
	};


	WuManber::WuManber(std::string files){
		trie = new Trie();

		mBlock = 3;
		int pattern_size = 0;
		mMin = 1024*1024;

		std::vector<std::string> strlist;

		std::string filename = files;
		std::ifstream ifs(filename, std::ifstream::in);
		while(!ifs.eof()) {
			std::string tmp;
			std::vector<std::string> tmp_vector;
			ifs >> tmp;

			boost::split(tmp_vector, tmp, boost::is_any_of("*"));		//Separates the *s

			for(int i = 0 ; i < tmp_vector.size(); i++){

				if(tmp_vector[i].length()==0) continue;					//Ignores and separates the ' 's
				if(tmp_vector[i].length() < mMin)
					mMin=tmp_vector[i].length();

				Pattern_Struct *pat = new Pattern_Struct();
				pat->whole_pattern = tmp;
				pat->this_pattern = tmp_vector[i];
				if(tmp_vector.size() == 1){
					pat->prev_pattern = "START";
					pat->next_pattern = "END";
				}
				else if(i == 0){
					pat->prev_pattern = "START";
					pat->next_pattern = tmp_vector[i+1];		//Creating the patterns.
				}
				else if(i == tmp_vector.size() - 1){
					pat->prev_pattern = tmp_vector[i-1];
					pat->next_pattern = "END";
				}
				else{
					pat->prev_pattern = tmp_vector[i-1];
					pat->next_pattern = tmp_vector[i+1];
				}
				trie->insert(*pat);
				strlist.push_back(tmp_vector[i]);
				pattern_size++;
			}
		}
		if(mBlock > mMin)
			mBlock = mMin;

		int32_t primes[6] = {1003, 10007, 100003, 1000003, 10000019, 100000007};
		std::vector<int32_t> primeList(&primes[0], &primes[6]);

		int32_t threshold = 10 * mBlock;
		int32_t mTableSize;

		for (size_t i = 0; i < primeList.size(); ++i) {
			if (primeList[i] > pattern_size && primeList[i] / pattern_size > threshold) {
				mTableSize = primeList[i];
				break;
			}
		}

		//if size of patternList is huge.
		if (0 == mTableSize) {
			std::cerr << "Warning: amount of pattern is very large, will cost a great amount of memory." << std::endl;
			mTableSize = primeList[5];
		}

		int32_t defaultValue = mMin - mBlock + 1;
		mShiftTable.resize(mTableSize, defaultValue);						//Do I need to Malloc here?
		table_size = mTableSize;

		for (int id = 0; id < pattern_size; ++id) {
		   // loop through each pattern from right to left
		   for (int index = mMin; index >= mBlock; --index) {
			   unsigned int hash = HashCode(strlist[id].c_str() + index - mBlock, mBlock) % mTableSize;
			   if (mShiftTable[hash] > (mMin - index)) {
				   mShiftTable[hash]  = mMin - index;
			   }
		   }
	   }
	}

	WuManber::~WuManber(){}


	std::vector<std::string> WuManber::search(std::string target){
		return search( target.c_str(), target.length());
	}

	std::vector<std::string> WuManber::search(const char *target, int length){
		Hash_Table *hash_t = new Hash_Table(3);
		int32_t index = mMin - 1;
		int32_t blockMaxIndex = mBlock-1;
		int32_t windowMaxIndex = mMin - 1;
		while(index < length){
			 unsigned int blockHash = HashCode(target + index - blockMaxIndex, mBlock);
			blockHash = blockHash % table_size;
			int shift = mShiftTable[blockHash];
			if (shift > 0)
				index += shift;
			else {
				trie->traverse(hash_t, target + index - mMin +1, index - mMin + 1);
				++index;
			}
		}
		std::vector<std::string> res = hash_t->compile();
		delete hash_t;
		return res;
	}

	int main(){
		std::string files;
		std::cout << "Enter filename: ";
		std::cin >> files;
		WuManber *wu = new WuManber(files);

		std::vector<std::string> res;
		std::string url;
		std::cout << "Enter target string: ";
		std::cin >> url;
	    struct timeval start, end;

		gettimeofday(&start, NULL);

		for(int i = 0; i < 1000; i++){
			res = wu->search(url);
		}

		gettimeofday(&end, NULL);


  		printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
		  - (start.tv_sec * 1000000 + start.tv_usec)));
  		for (int i = 0; i < res.size(); ++i) {
   			std::cout << res[i] << std::endl;
		}
		return 0;
	}



