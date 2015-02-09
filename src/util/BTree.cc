

// Structure of a Node
//
//	Node {
//		short	count;		//
//		char	status[B];
//		count_t values[B];
//		count_t children[B];
//		count_t next;
//  };
//



namespace Storage {

	
	struct Node {
		short count;
		char *status;
		count_t *values;
		count_t *children;
		count_t next;
	}
	
	Node &createNode( count_t B ) {
		Node n = new Node();
		n.status = new char[B];
		n.values = new count_t[B];
		n.children = new count_t[B];
		return n;
	}
	
	void deleteNode(Node &n) {
		delete[] n.children;
		delete[] n.values;
		delete[] n.status;
		delete n;
	}

	void readToken( Stream &s , count_t pos,  Node &n , count_t B ) {
		s.seek( pos );
		s.read( n.count , sizeof(count) );
		s.read( n.status , sizeof(char) * B );
		s.read( n.value , sizeof(count_t) * B );
		s.read( n.children , sizeof(count_t) * B );
		s.read( &n.count , sizeof(count_t) );
	}
	
	void getNode( count_t key , Stream &stream , count_t B  , Node &n) {
		count_t position = HEADER_BYTES;
		bool found = false;
		while( position != 0 && !found) {
			readNode( stream , position = n.children[i] , n , B );
			// TODO : Binary search
			for( int i = 0 ; i < count ; ++i ) {
				found = values[i] == key;
				if( n.values[i] > key ) {
					position = n.children[i];
					break;
				}
			}
		}
	}
	
	bool BTree::contains( count_t key ) {
		bool found = false;
		Node n = createNode( B );
		getNode( key , stream , B , n );
		// TODO : Binary search
		for( int i = 0 ; i < n.count && !found; ++i ) {
			if( n.value[i] == key ) found = true;
		}
		deleteNode( n );
		return found;
	}

	
	void BTree:insert( count_t key ) {
		Node n = createNode( B );
		getNode( key , stream , B , n );
		
		if( n.count != B - 1 ) {	// Not full
			// Find spot to place
			int i = 0;
			for( i < n.count; ++i ) {
				if( n.values[i] > key ) {
					break;
				}
			}
			
			// Copy down
			for( int j = n.count; j > i ; --j) {
				n.status[j] = n.status[j-1];
				n.values[j] = n.values[j-1];
				n.children[j] = n.children[j-1];
			}
			
			// update count
			n.count = n.count + 1;
			
			// Write my stuff!
			n.status[i] = EMPTY;
			n.values[i] = key;
			n.children[i] = 0;
			
			writeNode( stream , B , n );
		}else {						// full
			// Split all the way up?
			Node n2 = createNode( B );
			
			while( pos != HEADER_BYTES ) {				
				int c1 = count / 2;
				int c2 = count - c1;
				
				for( int i = c1 ; i < count ; ++i ) {
					n2.status[i - c1] = n.status[i];
				}
			}
			deleteNode( n2 );
		}
		
		deleteNode( n );
	}
	
	bool BTreee::remove( count_t key ) {
	}
	Record &retrieve( count_t key ) const;			// O(lg(n))
	
	
	bool contains( Criteria &c ) const;				// O(n)
	vector<Record> &retrieve( Criteria &c ) const;	// O(n)
	count_T	remove( Criteria &c );					// O(n)
	
	void count() const;			// How many records
	void bytes() const;			// How much space used
	
}