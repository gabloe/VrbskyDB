/*
 * STX B+ Tree Test Suite v0.9
 * Copyright (C) 2008-2013 Timo Bingmann
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <stx/btree_multiset.h>

#define ABORT() std::cout << "Aborted at: " << __FILE__ << ":" << __LINE__ << std::endl; return;
#define FAIL() std::cout << "Failed at: " << __FILE__ << ":" << __LINE__ << std::endl;
#define PASS()  /* do nothing */

#define ASSERT(condition); do { if(condition) { PASS(); } else { ABORT(); } } while(0)

template <typename KeyType>
struct traits_nodebug : stx::btree_default_set_traits<KeyType>
{
	static const bool       selfverify = true;
	static const bool       debug = false;

	static const int        leafslots = 8;
	static const int        innerslots = 8;
};


void test_dump_restore_3200()
{
	typedef stx::btree_multiset<unsigned int,
	    std::less<unsigned int>, traits_nodebug<unsigned int> > btree_type;

	{
	    btree_type bt;

	    srand(34234235);
	    for(unsigned int i = 0; i < 3200; i++)
	    {
		bt.insert(rand() % 100);
	    }

	    ASSERT(bt.size() == 3200);
	    
	    std::stringstream os;
	    std::ofstream out("btree.out", std::ofstream::out);

	    bt.dump(os);
	    out << os.rdbuf();
	    std::cout << "Wrote B+ tree to \"btree.out\"" << std::endl;
	    out.close();
	}

	// Also cannot check the length, because it depends on the rand()
	// algorithm in stdlib.
	// ASSERT( dumpstr.size() == 47772 );

	// cannot check the string with a hash function, because it contains
	// memory pointers

	{ // restore the btree image
	    btree_type bt2;

	    std::ifstream in("btree.out", std::ifstream::in);

	    std::stringstream iss;
	    iss << in.rdbuf();
	    in.close();

	    std::cout << "Reading \"btree.out\"" << std::endl;

	    ASSERT( bt2.restore(iss) );

	    ASSERT( bt2.size() == 3200 );

	    srand(34234235);
	    for(unsigned int i = 0; i < 3200; i++)
	    {
		ASSERT( bt2.exists(rand() % 100) );
	    }
	    std::cout << "PASSED!" << std::endl;
	}
}

int main(void) {
	test_dump_restore_3200();
	return 0;
}
