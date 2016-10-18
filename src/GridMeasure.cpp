//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Mon Oct 17 19:18:43 PDT 2016
// Filename:      GridMeasure.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridMeasure.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   GridMeasure stores the data for each measure in a HumGrid
//                object.
//

#include "HumGrid.h"

using namespace std;

namespace hum {


//////////////////////////////
//
// GridMeasure::GridMeasure -- Constructor.
//

GridMeasure::GridMeasure(void) {
	// do nothing;
}



//////////////////////////////
//
// GridMeasure::~GridMeasure -- Deconstructor.
//

GridMeasure::~GridMeasure(void) {
	for (auto it = this->begin(); it != this->end(); it++) {
		if (*it) {
			delete *it;
			*it = NULL;
		}
	}
}



//////////////////////////////
//
// GridMeasure::transferTokens --
//

void GridMeasure::transferTokens(HumdrumFile& outfile, bool recip) {
	HumNum linedur;
ggg do iteration over list rather than vector here...
	for (int i=0; i<(int)this->size(); i++) {
		if (i < (int)this->size() - 1) {
			linedur = this->at(i+1).starttime - this->at(i).starttime;
		} else {
			linedur = 55;
		}
		this->at(i)->transferTokens(outfile, recip, linedur);
	}
}

} // end namespace hum



