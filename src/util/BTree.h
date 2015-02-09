
#ifndef B_TREE_H_
#define B_TREE_H_



namespace Storage {
	
	// B+ Tree
	class BTree {
	
		public:
		
			void insert( count_t key );						// O(lg(n))
			bool contains( count_t key ) const;				// O(lg(n))
			bool remove( count_t key );						// O(lg(n))
			Record &retrieve( count_t key ) const;			// O(lg(n))
			
			
			bool contains( Criteria &c ) const;				// O(n)
			vector<Record> &retrieve( Criteria &c ) const;	// O(n)
			count_T	remove( Criteria &c );					// O(n)
			
			void count() const;			// How many records
			void bytes() const;			// How much space used
			
			static BTree &build( Stream &str , count_t b_factor );
			static BTree &open( Stream &str );
		private:
	}
}

#endif