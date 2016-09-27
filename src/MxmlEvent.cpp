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
// MxmlEvent::MxmlEvent --
//

MxmlEvent::MxmlEvent(MxmlMeasure* measure) {
	clear();
	m_owner = measure;
	m_sequence = m_counter++;
}



//////////////////////////////
//
// MxmlEvent::~MxmlEvent --
//

MxmlEvent::~MxmlEvent() {
	clear();
}



//////////////////////////////
//
// MxmlEvent::clear --
//

void MxmlEvent::clear(void) {
	m_starttime = m_duration = 0;
	m_eventtype = mevent_unknown;
	m_owner = NULL;
	m_linked = false;
	m_voice = m_staff = 0;
	m_links.clear();
}



//////////////////////////////
//
// MxmlEvent::setStartTime --
//

void MxmlEvent::setStartTime(HumNum value) {
	m_starttime = value;
}



//////////////////////////////
//
// MxmlEvent::setDuration --
//

void MxmlEvent::setDuration(HumNum value) {
	m_duration = value;
}



//////////////////////////////
//
// MxmlEvent::getStartTime --
//

HumNum MxmlEvent::getStartTime(void) const {
	return m_starttime;
}



//////////////////////////////
//
// MxmlEvent::getDuration --
//

HumNum MxmlEvent::getDuration(void) const {
	return m_duration;
}



//////////////////////////////
//
// MxmlEvent::setOwner --
//

void MxmlEvent::setOwner(MxmlMeasure* measure) {
	m_owner = measure;
}



//////////////////////////////
//
// MxmlEvent::getOwner --
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
// MxmlEvent::parseEvent --
//

bool MxmlEvent::parseEvent(xpath_node el) {
	return parseEvent(el.node());
}



//////////////////////////////
//
// MxmlEvent::getIntValue --
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
// Mxmlvent::setDurationByTicks --
//

void MxmlEvent::setDurationByTicks (long value) {
	long ticks = getQTicks();
	if (ticks == 0) {
		setDuration(0);
		return;
	}
	HumNum val = value;
	val /= ticks;
	setDuration(val);
}


//////////////////////////////
//
// MxmlEvent::hasChild --
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
// MxmlEvent::link --
//

void MxmlEvent::link(MxmlEvent* event) {
	m_links.push_back(event);
	event->setLinked();
}



//////////////////////////////
//
// MxmlEvent::setLinked --
//

void MxmlEvent::setLinked(void) {
	m_linked = true;
}



//////////////////////////////
//
// MxmlEvent::isLinked --
//

bool MxmlEvent::isLinked(void) const {
	return m_linked;
}



//////////////////////////////
//
// MxmlEvent::isChord --
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
// MxmlEvent::printEvent --
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
// MxmlEvent::getSequenceNumber --
//

int MxmlEvent::getSequenceNumber(void) const {
	return m_sequence;
}



//////////////////////////////
//
// MxmlEvent::getVoiceNumber --
//

int MxmlEvent::getVoiceNumber(void) const {
	return m_voice;
}



//////////////////////////////
//
// MxmlEvent::getStaffNumber --
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
// MxmlEvent::getType --
//

measure_event_type MxmlEvent::getType(void) const {
	return m_eventtype;
}



//////////////////////////////
//
// MxmlEvent::parseEvent --
//

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

	switch (m_eventtype) {
		case mevent_note:
			setDuration(0);
			if (hasChild("./chord")) {
				setDuration(0);
				attachToLastEvent();
			} else {
				setDurationByTicks(tempduration);
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


} // end namespace hum



