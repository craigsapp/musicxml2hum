// vim: ts=3

#ifndef _MXMLEVENT_H
#define _MXMLEVENT_H

#include "humlib.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

using namespace pugi;
using namespace std;
using namespace hum;

class MxmlMeasure;

////////////////////////////////////////////////////////////////////////////
//
// MxmlEvent -- an item contained in a measure
//

// Event types

enum measure_event_type {
	mevent_unknown,
	mevent_attributes,
	mevent_backup,
	mevent_barline,
	mevent_bookmark,
	mevent_direction,
	mevent_figured_bass,
	mevent_forward,
	mevent_grouping,
	mevent_harmony,
	mevent_link,
	mevent_note,
	mevent_print,
	mevent_sound
};


class MxmlEvent {
	public:
		              MxmlEvent          (MxmlMeasure* measure);
		             ~MxmlEvent          ();
		void          clear              (void);
		bool          parseEvent         (xml_node el);
		bool          parseEvent         (xpath_node el);
		void          setTickStart       (long value, long ticks);
		void          setTickDur         (long value, long ticks);
		void          setStartTime       (HumNum value);
		void          setDuration        (HumNum value);
		void          setDurationByTicks (long value);
		HumNum        getStartTime       (void) const;
		HumNum        getDuration        (void) const;
		void          setOwner           (MxmlMeasure* measure);
		MxmlMeasure*  getOwner           (void) const;
		const char*   getName            (void) const;
		int           setQTicks          (long value);
		long          getQTicks          (void) const;
		long          getIntValue        (const char* query) const;
		bool          hasChild           (const char* query) const;
		void          link               (MxmlEvent* event);
		bool          isLinked           (void) const;
		void          setLinked          (void);
		void          attachToLastEvent  (void);
		bool          isChord            (void) const;
		void          printEvent         (void) const;
		int           getSequenceNumber  (void) const;
		int           getVoiceNumber     (void);
		int           getStaffNumber     (void);
		void          setVoice           (int value);
		void          setStaff           (int value);
		measure_event_type getType       (void);
		int           getPartNumber      (void);

	protected:
		HumNum             starttime;
		HumNum             duration;
		measure_event_type eventtype;
		xml_node           node;
		MxmlMeasure*       owner;
		vector<MxmlEvent*> links;
		bool               linked;
		int                sequence;
		static int         counter;
		short              staff;
		short              voice;
};


#endif /* _MXMLEVENT_H */



