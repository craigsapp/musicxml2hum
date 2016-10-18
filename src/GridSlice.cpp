//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      GridSlice.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/GridSlice.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   GridSlice store a timestamp, a slice type and data for
//                all parts in the given slice.
//

#include "HumGrid.h"
#include "GridPart.h"

using namespace std;

namespace hum {


//////////////////////////////
//
// GridSlice::GridSlice -- Constructor.
//

GridSlice::GridSlice(HumNum timestamp, SliceType type) {
	m_timestamp = timestamp;
	m_type = type;
}



//////////////////////////////
//
// GridSlice::~GridSlice -- Deconstructor.
//

GridSlice::~GridSlice(void) {
	for (int i=0; i<(int)this->size(); i++) {
		if (this->at(i)) {
			delete this->at(i);
			this->at(i) = NULL;
		}
	}
}



//////////////////////////////
//
// GridSlice::createRecipTokenFromDuration --
//

HTp GridSlice::createRecipTokenFromDuration(HumNum duration) {
	HTp token;
	if (duration.getNumerator() == 0) {
		token = new HumdrumToken("g");
		return token;
	} else if (duration.getNumerator() == 1) {
		token = new HumdrumToken(to_string(duration.getDenominator()));
		return token;
	}
	string str = to_string(duration.getDenominator()) + "%" +
	         to_string(duration.getNumerator());
	token = new HumdrumToken(str);
	return token;

	// try to fit to one dot:

}



//////////////////////////////
//
// GridSlice::transferTokens -- Create a HumdrumLine and append it to
//    the data.
//

void GridSlice::transferTokens(HumdrumFile& outfile, bool recip,
		HumNum linedur) {

	HTp token;
	HumdrumLine* line = new HumdrumLine;

	if (recip) {
		if (isNoteSlice()) {
			token = createRecipTokenFromDuration(linedur);
		} else if (isClefSlice()) {
			token = new HumdrumToken("*");
		} else {
			token = new HumdrumToken("55");
		}
		line->appendToken(token);
	}

	// extract the Tokens from each part/staff
	int p; // part index
	int s; // staff index
	int v; // voice index

	for (p=(int)size()-1; p>=0; p--) {
		GridPart& part = *this->at(p);
		for (s=(int)part.size()-1; s>=0; s--) {
			GridStaff& staff = *part.at(s);
			if (staff.size() == 0) {
				// fix this later.  For now if there are no notes
				// on the staff, add a null token.  Fix so that 
				// all open voices are given null tokens.
				token = new HumdrumToken(".");
				line->appendToken(token);
			} else {
				for (v=0; v<staff.size(); v++) {
					if (staff.at(v)->getToken()) {
						line->appendToken(staff.at(v)->getToken());
					} else {
						token = new HumdrumToken(".");
						line->appendToken(token);
					}
				}
			}
		}
	}

	outfile.appendLine(line);
}



//////////////////////////////
//
// GridSlice::initializePartStaves --
//

void GridSlice::initializePartStaves(vector<MxmlPart>& partdata) {
	int i, j;
	if (this->size() > 0) {
		// strange that this should happen, but presume the data
		// needs to be deleted.
		for (int i=0; i<(int)this->size(); i++) {
			if (this->at(i)) {
				delete this->at(i);
				this->at(i) = NULL;
			}
		}
	}
	this->resize(partdata.size());

	for (i=0; i<(int)partdata.size(); i++) {
		this->at(i) = new GridPart;
		this->at(i)->resize(partdata[i].getStaffCount());
		for (j=0; j<(int)partdata[i].getStaffCount(); j++) {
			this->at(i)->at(j) = new GridStaff;
		}
	}

}



} // end namespace hum



