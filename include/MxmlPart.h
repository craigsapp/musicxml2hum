// vim: ts=3

#ifndef _MXMLPART_H
#define _MXMLPART_H

#include "humlib.h"

#include "MxmlMeasure.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <vector>

using namespace pugi;
using namespace std;
using namespace hum;

////////////////////////////////////////////////////////////////////////////

class MxmlPart {
	public:
		              MxmlPart             (void);
		             ~MxmlPart             ();
		void          clear                (void);
		bool          readPart             (const string& id, xml_node partdef, 
		                                    xml_node part);
		bool          addMeasure           (xml_node mel);
		bool          addMeasure           (xpath_node mel);
		int           measureCount         (void);
		MxmlMeasure*  getMeasure           (int index);
		int           getMeasureCount      (void);
		long          getQTicks            (void);
		int           setQTicks            (long value);
	   MxmlMeasure*  getPreviousMeasure   (MxmlMeasure* measure);
		HumNum        getDuration          (void);
		void          allocateSortedEvents (void);
		void          setPartNumber        (int number);
		int           getPartNumber        (void);

	protected:
		vector<MxmlMeasure*> measures;
		vector<long>         qtick;
		int                  partnum;
		
};

#endif /* _MXMLPART_H */



