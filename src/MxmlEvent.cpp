//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 14:16:18 PDT 2016
// Filename:      musicxml2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/MxmlEvent.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   MusicXML parsing abstraction for elements which are children
//                of the measure element.
//

#include "humlib.h"

#include "MxmlEvent.h"
#include "MxmlMeasure.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <stdlib.h>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>

using namespace pugi;
using namespace std;

namespace hum {

class MxmlMeasure;
class MxmlPart;

int MxmlEvent::m_counter = 0;

////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MxmlEvent::MxmlEvent -- Constructor.
//

MxmlEvent::MxmlEvent(MxmlMeasure* measure) {
	clear();
	m_owner = measure;
	m_sequence = m_counter++;
}



//////////////////////////////
//
// MxmlEvent::~MxmlEvent -- Destructor.
//

MxmlEvent::~MxmlEvent() {
	clear();
}



//////////////////////////////
//
// MxmlEvent::clear -- Clear any previous contents of the object.
//

void MxmlEvent::clear(void) {
	m_starttime = m_duration = 0;
	m_eventtype = mevent_unknown;
	m_owner = NULL;
	m_linked = false;
	m_voice = m_staff = 0;
	m_sequence = -1;
	m_links.clear();
}



//////////////////////////////
//
// MxmlEvent::setStartTime -- Set the starting timestamp of the event
//    in terms of quater notes since the start of the music.
//

void MxmlEvent::setStartTime(HumNum value) {
	m_starttime = value;
}



//////////////////////////////
//
// MxmlEvent::setDuration -- Set the duration of the event in terms
//   of quarter note durations.
//

void MxmlEvent::setDuration(HumNum value) {
	m_duration = value;
}



//////////////////////////////
//
// MxmlEvent::getStartTime -- Return the start time of the event in terms
//      of quarter notes since the start of the music.
//

HumNum MxmlEvent::getStartTime(void) const {
	return m_starttime;
}



//////////////////////////////
//
// MxmlEvent::getDuration -- Return the duration of the event in terms
//      of quarter note durations.
//

HumNum MxmlEvent::getDuration(void) const {
	return m_duration;
}



//////////////////////////////
//
// MxmlEvent::setOwner -- Indicate which measure the event belongs to.
//

void MxmlEvent::setOwner(MxmlMeasure* measure) {
	m_owner = measure;
}



//////////////////////////////
//
// MxmlEvent::getOwner -- Return the measure object that contains this
//     event.  If there is no owner, then returns NULL.
//

MxmlMeasure* MxmlEvent::getOwner(void) const {
	return m_owner;
}



//////////////////////////////
//
// MxmlEvent::getPartNumber --
//

int MxmlEvent::getPartNumber(void) const {
	if (!m_owner) {
		return 0;
	}
	return m_owner->getPartNumber();
}



//////////////////////////////
//
// MxmlEvent::getName --
//

const char* MxmlEvent::getName(void) const {
	return m_node.name();
}



//////////////////////////////
//
// MxmlEvent::setQTicks -- Set the number of ticks per quarter note.
//     Returns the number of times that the ticks has been set.
//     Returns 0 if the tick count is invalid.
//

int MxmlEvent::setQTicks(long value) {
	if (value <= 0) {
		return 0;
	}
	if (m_owner) {
		return m_owner->setQTicks(value);
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlEvent::getQTicks -- Get the number of ticks per quarter note.
//

long MxmlEvent::getQTicks(void) const {
	if (m_owner) {
		return m_owner->getQTicks();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlEvent::getIntValue -- Convenience function for an XPath query,
//    where the child text of the element should be interpreted as
//    an integer.
//

long MxmlEvent::getIntValue(const char* query) const {
	const char* val = m_node.select_node(query).node().child_value();
	if (strcmp(val, "") == 0) {
		return 0;
	} else {
		return atoi(val);
	}
}



//////////////////////////////
//
// Mxmlvent::setDurationByTicks -- Given a <duration> element tick
//    count, set the duration by dividing by the current quarter-note
//    duration tick count (from a prevailing attribute setting for
//    <divisions>).
//

void MxmlEvent::setDurationByTicks(long value, xml_node el) {
	long ticks = getQTicks();
	if (ticks == 0) {
		setDuration(0);
		return;
	}
	HumNum val = value;
	val /= ticks;

	if (el) {
		HumNum checkval = getEmbeddedDuration(el);
		if (checkval != val) {
			// cerr << "WARNING: True duration " << checkval << " does not match";
			// cerr << " tick duration (buggy data: " << val << ")" << endl;
			val = checkval;
		}
	}


	setDuration(val);
}



//////////////////////////////
//
// MxmlEvent::hasChild -- True if the given XPath query resulting
//      element has a child node.
//

bool MxmlEvent::hasChild(const char* query) const {
	xpath_node result = m_node.select_single_node(query);
	return !result.node().empty();
}



//////////////////////////////
//
// MxmlEvent::attachToLast --
//

void MxmlEvent::attachToLastEvent(void) {
	if (!m_owner) {
		return;
	}
	m_owner->attachToLastEvent(this);
}



//////////////////////////////
//
// MxmlEvent::link --  This function is used to link secondary 
//   elements to a primary one.  Currently only used for chord notes.
//   The first note of a chord will be stored in event lists, and 
//   secondary notes will be suppressed from the list and instead
//   accessed through the m_links structure.
//

void MxmlEvent::link(MxmlEvent* event) {
	m_links.push_back(event);
	event->setLinked();
}



//////////////////////////////
//
// MxmlEvent::setLinked -- Indicate that a note is a secondary
//     chord note.
//

void MxmlEvent::setLinked(void) {
	m_linked = true;
}



//////////////////////////////
//
// MxmlEvent::isLinked -- Returns true if the note is a secondary
//     chord note.
//

bool MxmlEvent::isLinked(void) const {
	return m_linked;
}



//////////////////////////////
//
// MxmlEvent::isChord -- Returns true if the event is the primary note
//    in a chord.
//

bool MxmlEvent::isChord(void) const {
	if ((m_links.size() > 0) && (strcmp(m_node.name(), "note") == 0)) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// MxmlEvent::printEvent -- Useful for debugging.
//

void MxmlEvent::printEvent(void) const {
	cout << getStartTime() << "\t" << getDuration() << "\t" << m_node.name();
	if (isChord()) {
		cout << "\tCHORD";
	}
	cout << endl;
}



//////////////////////////////
//
// MxmlEvent::getSequenceNumber -- Return the sequence number of the 
//   event in the input data file.  Useful for sorting items which 
//   occur at the same time.
//

int MxmlEvent::getSequenceNumber(void) const {
	return m_sequence;
}



//////////////////////////////
//
// MxmlEvent::getVoiceNumber -- Return the voice number of the event.
//

int MxmlEvent::getVoiceNumber(void) const {
	return m_voice;
}



//////////////////////////////
//
// MxmlEvent::getStaffNumber -- Return the staff number of the event.
//

int MxmlEvent::getStaffNumber(void) const {
	return m_staff;
}



//////////////////////////////
//
// MxmlEvent::setVoice --
//

void MxmlEvent::setVoice(int value) {
	m_voice = (short)value;
}



//////////////////////////////
//
// MxmlEvent::setStaff --
//

void MxmlEvent::setStaff(int value) {
	m_staff = (short)value;
}



//////////////////////////////
//
// MxmlEvent::getStaff -- 
//

int MxmlEvent::getStaff(void) const {
	if (!m_staff) {
		return 1;
	} else {
		return m_staff;
	}
}



//////////////////////////////
//
// MxmlEvent::getType --
//

measure_event_type MxmlEvent::getType(void) const {
	return m_eventtype;
}



//////////////////////////////
//
// MxmlEvent::parseEvent --
//

bool MxmlEvent::parseEvent(xpath_node el) {
	return parseEvent(el.node());
}


bool MxmlEvent::parseEvent(xml_node el) {
	m_node = el;

	if (strcmp(m_node.name(), "attributes") == 0) {
		m_eventtype = mevent_attributes;
	} else if (strcmp(m_node.name(), "backup") == 0) {
		m_eventtype = mevent_backup;
	} else if (strcmp(m_node.name(), "barline") == 0) {
		m_eventtype = mevent_barline;
	} else if (strcmp(m_node.name(), "bookmark") == 0) {
		m_eventtype = mevent_bookmark;
	} else if (strcmp(m_node.name(), "direction") == 0) {
		m_eventtype = mevent_direction;
	} else if (strcmp(m_node.name(), "figured-bass") == 0) {
		m_eventtype = mevent_figured_bass;
	} else if (strcmp(m_node.name(), "forward") == 0) {
		m_eventtype = mevent_forward;
	} else if (strcmp(m_node.name(), "grouping") == 0) {
		m_eventtype = mevent_grouping;
	} else if (strcmp(m_node.name(), "harmony") == 0) {
		m_eventtype = mevent_harmony;
	} else if (strcmp(m_node.name(), "link") == 0) {
		m_eventtype = mevent_link;
	} else if (strcmp(m_node.name(), "note") == 0) {
		m_eventtype = mevent_note;
	} else if (strcmp(m_node.name(), "print") == 0) {
		m_eventtype = mevent_print;
	} else if (strcmp(m_node.name(), "sound") == 0) {
		m_eventtype = mevent_sound;
	} else {
		m_eventtype = mevent_unknown;
	}

	int tempvoice    = 0;
	int tempstaff    = 0;
	int tempduration = 0;
	const char* tempname;

	for (auto el = m_node.first_child(); el; el = el.next_sibling()) {
		tempname = el.name();
		if (strcmp(tempname, "staff") == 0) {
			tempstaff = atoi(el.child_value());
		} else if (strcmp(tempname, "voice") == 0) {
			tempvoice = atoi(el.child_value());
		} else if (strcmp(tempname, "duration") == 0) {
			tempduration = atoi(el.child_value());
		}
	}

	m_voice = (short)tempvoice;
	m_staff = (short)tempstaff;
   reportStaffNumberToOwner(m_staff);

	switch (m_eventtype) {
		case mevent_note:
			setDuration(0);
			if (hasChild("./chord")) {
				setDuration(0);
				attachToLastEvent();
			} else {
				setDurationByTicks(tempduration, el);
			}
			break;

		case mevent_forward:
			setDurationByTicks(tempduration);
			break;

		case mevent_backup:
			setDurationByTicks(-tempduration);
			break;

		case mevent_attributes:
			setQTicks(getIntValue("./divisions"));
			break;

		case mevent_barline:
		case mevent_bookmark:
		case mevent_direction:
		case mevent_figured_bass:
		case mevent_grouping:
		case mevent_harmony:
		case mevent_link:
		case mevent_print:
		case mevent_sound:
		case mevent_unknown:
			setDuration(tempduration);
			break;
	}

	return true;
}



//////////////////////////////
//
// MxmlEvent::getRecip -- return **recip value for note/rest.
//

string MxmlEvent::getRecip(void) const {
	HumNum dur = m_duration;
	dur /= 4;  // convert to whole-note units;
	int n = getDotCount();
	if (n) {
		dur = dur * (1 << n) / ((1 << (n+1)) - 1);
	}
	stringstream ss;
	ss << dur.getDenominator();
	if (dur.getNumerator() != 1) {
		ss << "%" << dur.getNumerator();
	}
	for (int i=0; i<n; i++) {
		ss << ".";
	}
	return ss.str();
}



//////////////////////////////
//
// MxmlEvent::getKernPitch -- return **kern pitch of note/rest.
//

string MxmlEvent::getKernPitch(void) const {
	bool rest = false;
	xml_node child = m_node.first_child();

	string step;
	int alter  = 0;
	int octave = 4;

	while (child) {
		if (strcmp(child.name(), "rest") == 0) {
			rest = true;
			break;
		}
		if (strcmp(child.name(), "pitch") == 0) {
			xml_node grandchild = child.first_child();
			while (grandchild) {
				if (strcmp(grandchild.name(), "step") == 0) {
					step = grandchild.child_value();
				} else if (strcmp(grandchild.name(), "alter") == 0) {
					alter = atoi(grandchild.child_value());
				} else if (strcmp(grandchild.name(), "octave") == 0) {
					octave = atoi(grandchild.child_value());
				}
				grandchild = grandchild.next_sibling();
			}
		}
		child = child.next_sibling();
	}

	if (rest) {
		return "r";
	}

	int count = 1;
	char pc = 'C';
	if (step.size() > 0) {
		pc = step[0];
	}
	if (octave > 3) {
		pc = tolower(pc);
		count = octave - 3;
	} else {
		pc = toupper(pc);
		count = 4 - octave;
	}
	string output;
	for (int i=0; i<count; i++) {
		output += pc;
	}
	if (alter > 0) {  // sharps
		for (int i=0; i<alter; i++) {
			output += '#';
		}
	} else if (alter < 0) { // flats
		for (int i=0; i>alter; i--) {
			output += '-';
		}
	}
	// print cautionary natural sign here...

	return output;
}



//////////////////////////////
//
// MxmlEvent::getNode --
//

xml_node MxmlEvent::getNode(void) {
	return m_node;
}



///////////////////////////////////////////////////////////////////////////
//
// private functions --
//

//////////////////////////////
//
// MxmlEvent::reportStaffNumberToOwner --
//

void MxmlEvent::reportStaffNumberToOwner(int staffnum) {
	if (m_owner != NULL) {
		m_owner->receiveStaffNumberFromChild(staffnum);
	}
}



//////////////////////////////
//
//  MxmlEvent::getDotCount -- return the number of augmentation dots
//     which are children of the given event element.
//

int MxmlEvent::getDotCount(void) const {
	xml_node child = m_node.first_child();
	int output = 0;
	while (child) {
		if (output && (strcmp(child.name(), "dot") != 0)) {
			return output;
		}
		if (strcmp(child.name(), "dot") == 0) {
			output++;
		}
		child = child.next_sibling();
	}
	return output;
}



///////////////////////////////////////////////////////////////////////////
//
// static functions --
//

//////////////////////////////
//
// MxmlEvent::getEmbeddedDuration -- Given a <note>, return the
//   expeceded duration of the note, not from the <duration>, but
//   from a combination of <type> <dot>s and <time-modification>.
//   This value should match <duration>, but Sibelius has a buggy
//   <divisions> value so there can be round-off errors in the
//   duration of notes in MusicXML output from Sibelius.
//

HumNum MxmlEvent::getEmbeddedDuration(xml_node el) {
	if (!el) {
		return 0;
	}
	xml_node child = el.first_child();
   int dots          = 0;  // count of <dot /> elements
   HumNum type       = 0;  // powoftwo note type (as duration)
   bool tuplet       = false;  // is a tuplet
   int actualnotes   = 1;      // numerator of tuplet factor
   int normalnotes   = 1;      // denominator of tuplet factor
   HumNum normaltype = 0;      // poweroftwo duration of tuplet
   int tupdots       = 0;      // dots of "normal type" duration
	HumNum tfactor    = 1;

	while (child) {
		if (strcmp(child.name(), "dot") == 0) { 
			dots++;
		} else if (strcmp(child.name(), "type") == 0) {
			type = getQuarterDurationFromType(child.child_value());
		} else if (strcmp(child.name(), "time-modification") == 0) {
			xml_node grandchild = child.first_child();
			normaltype = type;
			tuplet = true;
			while (grandchild) {
				if (strcmp(grandchild.name(), "actual-notes") == 0) {
					actualnotes = atoi(grandchild.child_value());
				} else if (strcmp(grandchild.name(), "normal-notes") == 0) {
					normalnotes = atoi(grandchild.child_value());
				}
				grandchild = grandchild.next_sibling();
			}
         // no duration information after <time-modification> so exit
			// outer loop now.
			break;
		} else if (strcmp(child.name(), "normal-dot") == 0) {
			tupdots++;
		}
		child = child.next_sibling();
	}

	HumNum duration = type;
	if (dots) {
		HumNum newdur = duration;
		for (int i=0; i<dots; i++) {
			newdur += duration / (1 << (i+1));
		}
		duration = newdur;
	}
	if (tuplet) {
		HumNum modification(actualnotes, normalnotes);
		duration /= modification;
      if (normaltype != type) {
			cerr << "Warning: cannot handle this tuplet type yet" << endl;
		}
      if (tupdots != 0) {
			cerr << "Warning: cannot handle this tuplet dots yet" << endl;
		}
	}
	
	return duration;
}



////////////////////////////////////////
//
// MxmlEvent::getQuarterDurationFromType --
//

HumNum MxmlEvent::getQuarterDurationFromType(const char* type) {
	if      (strcmp(type, "quarter") == 0) { return 1;              } 
	else if (strcmp(type, "eighth") == 0)  { return HumNum(1, 2);   }
	else if (strcmp(type, "half") == 0)    { return 2;              }
	else if (strcmp(type, "16th") == 0)    { return HumNum(1, 4);   }
	else if (strcmp(type, "whole") == 0)   { return 4;              }
	else if (strcmp(type, "32nd") == 0)    { return HumNum(1, 8);   }
	else if (strcmp(type, "64th") == 0)    { return HumNum(1, 16);  }
	else if (strcmp(type, "128th") == 0)   { return HumNum(1, 32);  }
	else if (strcmp(type, "256th") == 0)   { return HumNum(1, 64);  }
	else if (strcmp(type, "512th") == 0)   { return HumNum(1, 128); }
	else if (strcmp(type, "1024th") == 0)  { return HumNum(1, 256); }
	else if (strcmp(type, "breve") == 0)   { return 8;              }
	else if (strcmp(type, "long") == 0)    { return 16;             }
	else if (strcmp(type, "maxima") == 0)  { return 32;             }
	else {
		cerr << "Error: Unknown note type: " << type << endl;
		return 0;
	}
}



} // end namespace hum



