//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      HumGrid.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/HumGrid.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//
//

#include "HumGrid.h"

using namespace std;

namespace hum {


//////////////////////////////
//
// HumGrid::HumGrid -- Constructor.
//

HumGrid::HumGrid(void) {
	// do nothing;
}



//////////////////////////////
//
// HumGrid::~HumGrid -- Deconstructor.
//

HumGrid::~HumGrid(void) {
	for (int i=0; i<(int)this->size(); i++) {
		if (this->at(i)) {
			delete this->at(i);
		}
	}
}



//////////////////////////////
//
// HumGrid::transferTokens --
//

void HumGrid::transferTokens(HumdrumFile& outfile) {
	insertStaffIndications(outfile);
	insertPartIndications(outfile);
	insertExclusiveInterpretationLine(outfile);
	int m; // measure index
	for (m=0; m<(int)this->size(); m++) {
		at(m)->transferTokens(outfile, m_recip);
		appendMeasureLine(outfile, *at(m)->back());
	}
	insertDataTerminationLine(outfile);
}



//////////////////////////////
//
// HumGrid::appendMeasureLine -- Hacky at the moment, deal with
//   subspine counts for each staff later. Assuming a single spine
//   for each staff at the moment.  Also calculate measure numbers.
//

void HumGrid::appendMeasureLine(HumdrumFile& outfile, GridSlice& slice) {
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("=");
		line->appendToken(token);
	}

	int p; // part index
	int s; // staff index

	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			token = new HumdrumToken("=");
			line->appendToken(token);
		}
	}
	outfile.appendLine(line);
}



//////////////////////////////
//
// HumGrid::insertExclusiveInterpretationLine -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.
//

void HumGrid::insertExclusiveInterpretationLine(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}

	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("**recip");
		line->appendToken(token);
	}

	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			token = new HumdrumToken("**kern");
			line->appendToken(token);
		}
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertPartIndications -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.  This is the
//    MusicXML Part number. (Some parts will contain more than one
//    staff).
//

void HumGrid::insertPartIndications(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*");
		line->appendToken(token);
	}

	string text;
	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			text = "*part" + to_string(p+1);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertStaffIndications -- Currently presumes
//    that the first entry contains spines.  And the first measure
//    in the HumGrid object must contain a slice.  This is the
//    MusicXML Part number. (Some parts will contain more than one
//    staff).
//

void HumGrid::insertStaffIndications(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}

	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*");
		line->appendToken(token);
	}

	string text;
	GridSlice& slice = *this->at(0)->front();
	int p; // part index
	int s; // staff index

	int staffcount = 0;
	for (p=0; p<(int)slice.size(); p++) {
		GridPart& part = *slice[p];
		staffcount += (int)part.size();
	}

	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			text = "*staff" + to_string(staffcount--);
			token = new HumdrumToken(text);
			line->appendToken(token);
		}
	}
	outfile.insertLine(0, line);
}



//////////////////////////////
//
// HumGrid::insertDataTerminationLine -- Currently presumes
//    that the last entry contains spines.  And the first 
//    measure in the HumGrid object must contain a slice.
//    Also need to compensate for *v on previous line.
//

void HumGrid::insertDataTerminationLine(HumdrumFile& outfile) {
	if (this->size() == 0) {
		return;
	}
	if (this->at(0)->empty()) {
		return;
	}
	HumdrumLine* line = new HumdrumLine;
	HTp token;

	if (m_recip) {
		token = new HumdrumToken("*-");
		line->appendToken(token);
	}

	GridSlice& slice = *this->at(0)->back();
	int p; // part index
	int s; // staff index
	for (p=(int)slice.size()-1; p>=0; p--) {
		GridPart& part = *slice[p];
		for (s=(int)part.size()-1; s>=0; s--) {
			token = new HumdrumToken("*-");
			line->appendToken(token);
		}
	}
	outfile.appendLine(line);
}


} // end namespace hum



