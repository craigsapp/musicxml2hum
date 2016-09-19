// vim: ts=3

#include "humlib.h"

#include "MxmlMeasure.h"
#include "MxmlPart.h"

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


////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MxmlPart::MxmlPart --
//

MxmlPart::MxmlPart(void) {
	clear();
}



//////////////////////////////
//
// MxmlPart::~MxmlPart --
//

MxmlPart::~MxmlPart(void) {
	clear();
}



//////////////////////////////
//
// MxmlPart::clear --
//


void MxmlPart::clear(void) {
	for (int i=0; i<(int)measures.size(); i++) {
		delete measures[i];
		measures[i] = NULL;
	}
	measures.clear();
	partnum = 0;
}



//////////////////////////////
//
// MxmlPart::getQTicks --
//

long MxmlPart::getQTicks(void) {
	if (qtick.size() > 0) {
		return qtick.back();
	} else {
		return 0;
	}
}



//////////////////////////////
//
// MxmlPart::setQTicks --
//

int MxmlPart::setQTicks(long value) {
	if (value < 0) {
		return (int)qtick.size();
	}
	if (qtick.size() > 0) {
		if (qtick.back() == value) {
			return (int)qtick.size();
		}
	}
	qtick.push_back(value);
	return (int)qtick.size();
}



//////////////////////////////
//
// MxmlPart::addMeasure --
//

bool MxmlPart::addMeasure(xpath_node mel) {
	return addMeasure(mel.node());
}


bool MxmlPart::addMeasure(xml_node mel) {
	MxmlMeasure* meas = new MxmlMeasure(this);
	if (measures.size() > 0) {
		meas->setPreviousMeasure(measures.back());
		measures.back()->setNextMeasure(meas);
	}
	measures.push_back(meas);
	return meas->parseMeasure(mel);
}



//////////////////////////////
//
// MxmlPart::measureCount --
//

int MxmlPart::measureCount(void) {
	return (int)measures.size();
}



//////////////////////////////
//
// MxmlPart::getMeasure --
//

MxmlMeasure* MxmlPart::getMeasure(int index) {
	if ((index < 0) || (index >= (int)measures.size())) {
		return NULL;
	}
	return measures[index];
}



//////////////////////////////
//
// MxmlPart::getPreviousMeasure --
//

MxmlMeasure* MxmlPart::getPreviousMeasure(MxmlMeasure* measure) {
	if (!measure) {
		return NULL;
	}
	if (measure == *measures.begin()) {
		return NULL;
	}
	if (measures.size() == 0) {
		return NULL;
	}

	MxmlMeasure* lastmeasure = measures[0];
	for (int i=1; i<(int)measures.size(); i++) {
		if (measure == measures[i]) {
			return lastmeasure;
		} else {
			lastmeasure = measures[i];
		}
	}
	return NULL;
}



//////////////////////////////
//
// MxmlPart::getDuration --
//

HumNum MxmlPart::getDuration(void) {
	if (measures.size() == 0) {
		return 0;
	}
	return measures.back()->getStartTime() + measures.back()->getDuration();
}



//////////////////////////////
//
// MxmlPart::setPartNumber --
//

void MxmlPart::setPartNumber(int number) { 
	partnum = number;
}



//////////////////////////////
//
// MxmlPart::getPartNumber --
//

int MxmlPart::getPartNumber(void) { 
	return partnum;
}




