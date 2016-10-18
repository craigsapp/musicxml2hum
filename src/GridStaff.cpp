//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      GridStaff.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridStaff.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   GridStaff is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#include "HumGrid.h"

using namespace std;

namespace hum {


//////////////////////////////
//
// GridStaff::GridStaff -- Constructor.
//

GridStaff::GridStaff(void) : vector<HTp>(0) {
	// do nothing;
}

//////////////////////////////
//
// GridStaff::~GridStaff -- Deconstructor.
//

GridStaff::~GridStaff(void) {
	// do nothing: for the moment;
	// all HumdrumTokens* stored in this
	// structure should be transferred into
	// a HumdrumFile structure.
}


//////////////////////////////
//
// GridStaff::setTokenLayer -- Insert a token at the given voice/layer index.
//    If there is another token already there, then delete it.  If there
//    is no slot for the given voice, then create one and fill in all of the
//    other new ones with NULLs.
//

void GridStaff::setTokenLayer(int layerindex, HTp token) {
	if (layerindex > (int)this->size()-1) {
		int oldsize = this->size();
		this->resize(layerindex+1);
		for (int i=oldsize; i<(int)this->size(); i++) {
			this->at(i) = NULL;
		}
	}
	if (this->at(layerindex) != NULL) {
		delete this->at(layerindex);
	}
	this->at(layerindex) = token;
}



//////////////////////////////
//
// GridStaff::appendTokenLayer -- concatenate the string content
//   of a token onto the current token stored in the slot (or just
//   place this one in the slot if none there yet).  This is used for
//   chords normally.
//

void GridStaff::appendTokenLayer(int layerindex, HTp token,
		const string& spacer) {

	if (layerindex > (int)this->size()-1) {
		int oldsize = this->size();
		this->resize(layerindex+1);
		for (int i=oldsize; i<(int)this->size(); i++) {
			this->at(i) = NULL;
		}
	}
	if (this->at(layerindex) != NULL) {
		string newtoken;
		newtoken = (string)*this->at(layerindex);
		newtoken += spacer;
		newtoken += (string)*token;
		(string)*(this->at(layerindex)) = newtoken;
	} else {
		this->at(layerindex) = token;
	}
}


} // end namespace hum



