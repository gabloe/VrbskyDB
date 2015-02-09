

namespace Storage {

	// Stream position enum used to calculate positions
	// related to these values
	enum F_POS { S_BEGIN , S_CURRENT , S_END };

	class Stream {
		
		// Move to a position in the file calculated using the offset
		// and where to calculate the offset from
		void seek( long int offset , S_POS from );
		
		// Move to the beginning of the stream
		void reset();
		
		// Write to the file length bytes
		std::size_t write( char *data , std::size_t length );
		
		// Try to read length bytes, returns how many bytes were actually read
		std::size_t read( char* data , std::size_t length );
	}
}