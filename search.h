#ifndef __SEARCH_H__
#define __SEARCH_H__

	typedef struct pt_struct {										//Pattern_Structs are basically four tuples of strings
		std::string whole_pattern;									//The whole_pattern represents the overall pattern, using example A*B, it would be A*B
		std::string prev_pattern;									//The prev_pattern represents the previous pattern and is used only for examples like A*B. B would have a prev of A, A would have a prev of START
		std::string next_pattern;									//The next_pattern is the counterpart to prev_pattern. Stores the next supposed pattern. A would have a next of B, B would have a next of END.
		std::string this_pattern;									//This pattern is the actual pattern itself. We can check: Does A's next = B and B's prev =A? We need both because if we only had one, we could get
																	//the example B, see it has an end and return the pattern B, when A has not occurred. We must check if B has a prev too.
	} Pattern_Struct;

	typedef std::vector<Pattern_Struct> Pattern_List;				//Vector of pointers to Pattern_Structs so they can be used for storing a dynamic blah.

	static unsigned int HashCode(const char* str);					//All of the static function declarations.
	static unsigned int HashCode(const char* str, int len);


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
		std::vector<Pattern_Struct> *table;

		void resize();

	};


	class Trie{
	public:
		Trie();
		~Trie();
		void insert(Pattern_Struct pat);
		void insert(const char *target, Pattern_Struct pat);
		void traverse(Hash_Table *ht, const char *target);

	private:
		Trie *array[128];											//Pointer to 128 other Tries
		Pattern_List pattern;										//Pattern_Struct if this Trie node is a match. This is a vector in case there are multiple patterns.
		bool match;													//Boolean used to determine if this node is a match
	};

	class WuManber{
		private:
			int table_size;
			int mBlock;
			int mMin;
			std::vector<int32_t> mShiftTable;
			Trie *trie;

			std::vector<std::string> search(const char *target, int length);

		public:
			WuManber(std::string files);
			~WuManber();
			std::vector<std::string> search(std::string target);
	};
#endif