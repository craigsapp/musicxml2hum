//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      HumGrid.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumGrid.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//

#ifndef _HUMGRID_H
#define _HUMGRID_H

#include "humlib.h"
#include "GridMeasure.h"

#include <vector>

using namespace std;


namespace hum {


class HumGrid : public vector<GridMeasure*> {
	public:
		HumGrid(void);
		~HumGrid();
		void transferTokens(HumdrumFile& outfile);

	protected:
		void insertExclusiveInterpretationLine (HumdrumFile& outfile);
		void insertDataTerminationLine         (HumdrumFile& outfile);
		void appendMeasureLine						(HumdrumFile& outfile,
		                                        GridSlice& slice);
		void insertPartIndications             (HumdrumFile& outfile);
		void insertStaffIndications            (HumdrumFile& outfile);

};


} // end namespace hum

#endif /* _HUMGRID_H */



