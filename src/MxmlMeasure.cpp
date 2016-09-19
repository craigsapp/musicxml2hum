// vim: ts=3

// measure element:
// http://usermanuals.musicxml.com/MusicXML/Content/EL-MusicXML-measure.htm

#include "humlib.h"

#include "MxmlEvent.h"
#include "MxmlMeasure.h"
#include "MxmlPart.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace pugi;
using namespace std;
using namespace hum;

class MxmlPart;

////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MxmlMeasure::MxmlMeasure --
//

MxmlMeasure::MxmlMeasure(MxmlPart* part) {
	clear();
	setOwner(part);
}



//////////////////////////////
//
// MxmlMeasure::~MxmlMeasure --
//

MxmlMeasure::~MxmlMeasure() {
	clear();
}



//////////////////////////////
//
// MxmlMeasure::clear --
//

void MxmlMeasure::clear(void) {
	starttime = duration = 0;
	for (int i=0; i<(int)events.size(); i++) {
		delete events[i];
		events[i] = NULL;
	}
	events.clear();
	owner = NULL;
	previous = following = NULL;
}



//////////////////////////////
//
// MxmlMeasure::parseMeasure --
//

bool MxmlMeasure::parseMeasure(xpath_node mel) {
	return parseMeasure(mel.node());
}


bool MxmlMeasure::parseMeasure(xml_node mel) {
	bool output = true;
	for (auto el = mel.first_child(); el; el = el.next_sibling()) {
		MxmlEvent* event = new MxmlEvent(this);
		events.push_back(event);
		output &= event->parseEvent(el);
	}
	
	setStartTime();
	calculateDuration();

	cout << endl;
	cout << "MEASURE DURATION: " << getDuration() << endl;
	for (int i=0; i<(int)events.size(); i++) {
		events[i]->printEvent();
	}

	sortEvents();

	return output;
}



//////////////////////////////
//
// MxmlMeasure::sortEvents --
//

void MxmlMeasure::sortEvents(void) {
	int i;
	set<HumNum> times;

	for (i=0; i<(int)events.size(); i++) {

		// skip storing certain types of events:
		switch (events[i]->getType()) {
			case mevent_forward:
			case mevent_backup:
				continue;
			case mevent_note:
				if (events[i]->isChord()) {
					continue;
				}
				break;
			default:
				break;
		}

		times.insert(events[i]->getStartTime());
	}

	sortedevents.resize(times.size());
	int counter = 0;

	for (HumNum val : times) {
		sortedevents[counter++].starttime = val;
	}

	// setup sorted access:
	map<HumNum, SimultaneousEvents*> mapping;
	for (i=0; i<(int)sortedevents.size(); i++) {
		mapping[sortedevents[i].starttime] = &sortedevents[i];
	}

	HumNum duration;
	HumNum starttime;
	for (i=0; i<(int)events.size(); i++) {

		// skip storing certain types of events:
		switch (events[i]->getType()) {
			case mevent_forward:
			case mevent_backup:
				continue;
			case mevent_note:
				if (events[i]->isChord()) {
					continue;
				}
				break;
			default:
				break;
		}

		starttime = events[i]->getStartTime();
		duration  = events[i]->getDuration();
		if (duration == 0) {
			mapping[starttime]->zerodur.push_back(events[i]);
		} else {
			mapping[starttime]->nonzerodur.push_back(events[i]);
		}
	}

	
	int j;
	vector<SimultaneousEvents>& se = sortedevents;
	cout << "TIME SORTED EVENTS:" << endl;
	for (i=0; i<(int)se.size(); i++) {
		if (se[i].zerodur.size() > 0) {
			cout << se[i].starttime << "z\t";
			for (j=0; j<(int)se[i].zerodur.size(); j++) {
				cout << " " << se[i].zerodur[j]->getName();
				cout << "(";
				cout << se[i].zerodur[j]->getPartNumber();
				cout << ",";
				cout << se[i].zerodur[j]->getStaffNumber();
				cout << ",";
				cout << se[i].zerodur[j]->getVoiceNumber();
				cout << ")";
			}
			cout << endl;
		}
		if (se[i].nonzerodur.size() > 0) {
			cout << se[i].starttime << "\t";
			for (j=0; j<(int)se[i].nonzerodur.size(); j++) {
				cout << " " << se[i].nonzerodur[j]->getName();
				cout << "(";
				cout << se[i].nonzerodur[j]->getPartNumber();
				cout << ",";
				cout << se[i].nonzerodur[j]->getStaffNumber();
				cout << ",";
				cout << se[i].nonzerodur[j]->getVoiceNumber();
				cout << ")";
			}
			cout << endl;
		}
	}

}



//////////////////////////////
//
// MxmlMeasure::setStartTime --
//

void MxmlMeasure::setStartTime(void) {
	if (!owner) {
		setStartTime(0);
		return;
	}
	MxmlMeasure* previous = owner->getPreviousMeasure(this);
	if (!previous) {
		setStartTime(0);
		return;
	}
	setStartTime(previous->getStartTime() + previous->getDuration());
}



//////////////////////////////
//
// MxmlMeasure::calculateDuration --
//

void MxmlMeasure::calculateDuration(void) {
	HumNum maxdur   = 0;
	HumNum sum      = 0;
	HumNum chorddur = 0;
	for (int i=0; i<(int)events.size(); i++) {
		events[i]->setStartTime(sum + getStartTime());
		if (events[i]->isChord()) {
			events[i]->setStartTime(events[i]->getStartTime() - chorddur);
		} else {
			chorddur = events[i]->getDuration();
		}
		sum += events[i]->getDuration();
		if (maxdur < sum) {
			maxdur = sum;
		}
	}
	setDuration(maxdur);
}



//////////////////////////////
//
// MxmlMeasure::setStartTime --
//

void MxmlMeasure::setStartTime(HumNum value) {
	starttime = value;
}



//////////////////////////////
//
// MxmlMeasure::setDuration --
//

void MxmlMeasure::setDuration(HumNum value) {
	duration = value;
}



//////////////////////////////
//
// MxmlMeasure::getStartTime --
//

HumNum MxmlMeasure::getStartTime(void) {
	return starttime;
}



//////////////////////////////
//
// MxmlMeasure::getDuration --
//

HumNum MxmlMeasure::getDuration(void) {
	return duration;
}



//////////////////////////////
//
// MxmlMeasure::setOwner --
//

void MxmlMeasure::setOwner(MxmlPart* part) {
	owner = part;
}



//////////////////////////////
//
// MxmlMeasure::setOwner --
//

MxmlPart* MxmlMeasure::getOwner(void) {
	return owner;
}



//////////////////////////////
//
// MxmlMeasure::getPartNumber --
//

int MxmlMeasure::getPartNumber(void) {
	if (!owner) {
		return 0;
	}
	return owner->getPartNumber();
}



//////////////////////////////
//
// MxmlMeasure::setQTicks -- Set the number of ticks per quarter note.
//     Returns the number of times that the ticks has been set.
//

int MxmlMeasure::setQTicks(long value) {
	if (owner) {
		return owner->setQTicks(value);
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlMeasure::getQTicks -- Get the number of ticks per quarter note.
//

long MxmlMeasure::getQTicks(void) {
	if (owner) {
		return owner->getQTicks();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlMeasure::attachToLastEvent --
//

void MxmlMeasure::attachToLastEvent(MxmlEvent* event) {
	if (events.size() == 0) {
		return;
	}
	events.back()->link(event);
}



//////////////////////////////
//
// MxmlMeasure::getEventCount --
//

int MxmlMeasure::getEventCount(void) {
	return (int)events.size();
}



//////////////////////////////
//
// MxmlMeasure::getEvent --
//

MxmlEvent* MxmlMeasure::getEvent(int index) {
	if (index < 0) {
		return NULL;
	}
	if (index >= (int)events.size()) {
		return NULL;
	}
	return events[index];
}



//////////////////////////////
//
// MxmlMeasure::setPreviousMeasure --
//

void MxmlMeasure::setPreviousMeasure(MxmlMeasure* event) {
	previous = event;
}



//////////////////////////////
//
// MxmlMeasure::setNextMeasure --
//

void MxmlMeasure::setNextMeasure(MxmlMeasure* event) {
	following = event;
}



//////////////////////////////
//
// MxmlMeasure::getPreviousMeasure --
//

MxmlMeasure* MxmlMeasure::getPreviousMeasure(void) {
	return previous;
}



//////////////////////////////
//
// MxmlMeasure::getNextMeasure --
//

MxmlMeasure* MxmlMeasure::getNextMeasure(void) {
	return following;
}



