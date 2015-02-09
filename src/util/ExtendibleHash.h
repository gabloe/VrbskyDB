

namespace Storage {

	class ExtendibleHashing {
	
		Record &get( count_t ) const;
		&set( count_t key , Record &r );
	
		ExtendibleHash &build();
		ExtendibleHash &open();
	}
}