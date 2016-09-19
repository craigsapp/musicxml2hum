// vim: ts=3
//
// measure element:
// http://usermanuals.musicxml.com/MusicXML/Content/EL-MusicXML-measure.htm

#ifndef _MXMLMEASURE_H
#define _MXMLMEASURE_H

#include "humlib.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <vector>

using namespace pugi;
using namespace std;
using namespace hum;

class MxmlEvent;
class MxmlPart;


class SimultaneousEvents {
	public:
		SimultaneousEvents(void) { }
		~SimultaneousEvents() { }
		HumNum starttime;
		vector<MxmlEvent*> zerodur;
		vector<MxmlEvent*> nonzerodur;
};


////////////////////////////////////////////////////////////////////////////
//
// MxmlMeasure -- a measure.
//

class MxmlMeasure {
	public:
		              MxmlMeasure        (MxmlPart* part);
		             ~MxmlMeasure        (void);
		void          clear              (void);
		bool          parseMeasure       (xml_node mel);
		bool          parseMeasure       (xpath_node mel);
		void          setStartTime       (HumNum value);
		void          setStartTime       (void);
		void          setDuration        (HumNum value);
		HumNum        getStartTime       (void);
		HumNum        getDuration        (void);
		void          setOwner           (MxmlPart* part);
		MxmlPart*     getOwner           (void);
		int           getPartNumber      (void);
		int           setQTicks          (long value);
		long          getQTicks          (void);
		void          attachToLastEvent  (MxmlEvent* event);
		void          calculateDuration  (void);
		int           getEventCount      (void);
		MxmlEvent*    getEvent           (int index);
		void          setPreviousMeasure (MxmlMeasure* event);
		void          setNextMeasure     (MxmlMeasure* event);
		MxmlMeasure*  getPreviousMeasure (void);
		MxmlMeasure*  getNextMeasure     (void);

	private:
		void          sortEvents         (void);

	protected:
		HumNum             starttime; // start time of measure in quarter notes
		HumNum             duration;  // duration of measure in quarter notes
		MxmlPart*          owner;     // part which contains measure
		MxmlMeasure*       previous;  // previous measure in part or null
		MxmlMeasure*       following; // following measure in part or null
		vector<MxmlEvent*> events;    // list of semi-ordered events in measure
		vector<SimultaneousEvents> sortedevents; // list of time-sorted events
};



#endif /* _MXMLMEASURE_H */



