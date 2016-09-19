// vim: ts=3

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
using namespace hum;

class MxmlMeasure;
class MxmlPart;

int MxmlEvent::counter = 0;

////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MxmlEvent::MxmlEvent --
//

MxmlEvent::MxmlEvent(MxmlMeasure* measure) {
	clear();
	owner = measure;
	sequence = counter++;
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
	starttime = duration = 0;
	eventtype = mevent_unknown;
	owner = NULL;
	linked = false;
	voice = staff = 0;
	links.clear();
}



//////////////////////////////
//
// MxmlEvent::setStartTime --
//

void MxmlEvent::setStartTime(HumNum value) {
	starttime = value;
}



//////////////////////////////
//
// MxmlEvent::setDuration --
//

void MxmlEvent::setDuration(HumNum value) {
	duration = value;
}



//////////////////////////////
//
// MxmlEvent::getStartTime --
//

HumNum MxmlEvent::getStartTime(void) const {
	return starttime;
}



//////////////////////////////
//
// MxmlEvent::getDuration --
//

HumNum MxmlEvent::getDuration(void) const {
	return duration;
}



//////////////////////////////
//
// MxmlEvent::setOwner --
//

void MxmlEvent::setOwner(MxmlMeasure* measure) {
	owner = measure;
}



//////////////////////////////
//
// MxmlEvent::getOwner --
//

MxmlMeasure* MxmlEvent::getOwner(void) const {
	return owner;
}



//////////////////////////////
//
// MxmlEvent::getPartNumber --
//

int MxmlEvent::getPartNumber(void) {
	if (!owner) {
		return 0;
	}
	return owner->getPartNumber();
}



//////////////////////////////
//
// MxmlEvent::getName --
//

const char* MxmlEvent::getName(void) const {
	return node.name();
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
	if (owner) {
		return owner->setQTicks(value);
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlEvent::getQTicks -- Get the number of ticks per quarter note.
//

long MxmlEvent::getQTicks(void) const {
	if (owner) {
		return owner->getQTicks();
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
	const char* val = node.select_node(query).node().child_value();
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
	xpath_node result = node.select_single_node(query);
	return !result.node().empty();
}



//////////////////////////////
//
// MxmlEvent::attachToLast --
//

void MxmlEvent::attachToLastEvent(void) {
	if (!owner) {
		return;
	}
	owner->attachToLastEvent(this);
}



//////////////////////////////
//
// MxmlEvent::link --
//

void MxmlEvent::link(MxmlEvent* event) {
	links.push_back(event);
	event->setLinked();
}



//////////////////////////////
//
// MxmlEvent::setLinked --
//

void MxmlEvent::setLinked(void) {
	linked = true;
}



//////////////////////////////
//
// MxmlEvent::isLinked --
//

bool MxmlEvent::isLinked(void) const {
	return linked;
}



//////////////////////////////
//
// MxmlEvent::isChord --
//

bool MxmlEvent::isChord(void) const {
	if ((links.size() > 0) && (strcmp(node.name(), "note") == 0)) {
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
	cout << getStartTime() << "\t" << getDuration() << "\t" << node.name();
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
	return sequence;
}



//////////////////////////////
//
// MxmlEvent::getVoiceNumber --
//

int MxmlEvent::getVoiceNumber(void) {
	return voice;
}



//////////////////////////////
//
// MxmlEvent::getStaffNumber --
//

int MxmlEvent::getStaffNumber(void) {
	return staff;
}



//////////////////////////////
//
// MxmlEvent::setVoice --
//

void MxmlEvent::setVoice(int value) {
	voice = (short)value;
}



//////////////////////////////
//
// MxmlEvent::setStaff --
//

void MxmlEvent::setStaff(int value) {
	staff = (short)value;
}



//////////////////////////////
//
// MxmlEvent::getType --
//

measure_event_type MxmlEvent::getType(void) {
	return eventtype;
}



//////////////////////////////
//
// MxmlEvent::parseEvent --
//

bool MxmlEvent::parseEvent(xml_node el) {
	node = el;

	if (strcmp(node.name(), "attributes") == 0) {
		eventtype = mevent_attributes;
	} else if (strcmp(node.name(), "backup") == 0) {
		eventtype = mevent_backup;
	} else if (strcmp(node.name(), "barline") == 0) {
		eventtype = mevent_barline;
	} else if (strcmp(node.name(), "bookmark") == 0) {
		eventtype = mevent_bookmark;
	} else if (strcmp(node.name(), "direction") == 0) {
		eventtype = mevent_direction;
	} else if (strcmp(node.name(), "figured-bass") == 0) {
		eventtype = mevent_figured_bass;
	} else if (strcmp(node.name(), "forward") == 0) {
		eventtype = mevent_forward;
	} else if (strcmp(node.name(), "grouping") == 0) {
		eventtype = mevent_grouping;
	} else if (strcmp(node.name(), "harmony") == 0) {
		eventtype = mevent_harmony;
	} else if (strcmp(node.name(), "link") == 0) {
		eventtype = mevent_link;
	} else if (strcmp(node.name(), "note") == 0) {
		eventtype = mevent_note;
	} else if (strcmp(node.name(), "print") == 0) {
		eventtype = mevent_print;
	} else if (strcmp(node.name(), "sound") == 0) {
		eventtype = mevent_sound;
	} else {
		eventtype = mevent_unknown;
	}

	int tempvoice    = 0;
	int tempstaff    = 0;
	int tempduration = 0;
	const char* tempname;

	for (auto el = node.first_child(); el; el = el.next_sibling()) {
		tempname = el.name();
		if (strcmp(tempname, "staff") == 0) {
			tempstaff = atoi(el.child_value());
		} else if (strcmp(tempname, "voice") == 0) {
			tempvoice = atoi(el.child_value());
		} else if (strcmp(tempname, "duration") == 0) {
			tempduration = atoi(el.child_value());
		}
	}

	voice = (short)tempvoice;
	staff = (short)tempstaff;

	switch (eventtype) {
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


