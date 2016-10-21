//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 14:16:18 PDT 2016
// Filename:      musicxml2hum.cpp
// URL:           https://github.com/craigsapp/hum2ly/blob/master/src/musicxml2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert a MusicXML file into a Humdrum file.
//

#include "musicxml2hum.h"
#include "HumGrid.h"

using namespace std;
using namespace pugi;

namespace hum {


//////////////////////////////
//
// MusicXmlToHumdrumConverter::MusicXmlToHumdrumConverter --
//

MusicXmlToHumdrumConverter::MusicXmlToHumdrumConverter(void){
	// Options& options = m_options;
	// options.define("k|kern=b","display corresponding **kern data");
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::convert -- Convert a MusicXML file into
//     Humdrum content.
//

bool MusicXmlToHumdrumConverter::convertFile(ostream& out,
		const char* filename) {
	xml_document doc;
	auto result = doc.load_file(filename);
	if (!result) {
		cerr << "\nXML file [" << filename << "] has syntax errors\n";
		cerr << "Error description:\t" << result.description() << "\n";
		cerr << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}


bool MusicXmlToHumdrumConverter::convert(ostream& out, istream& input) {
	string s(istreambuf_iterator<char>(input), {});
	return convert(out, s.c_str());
}


bool MusicXmlToHumdrumConverter::convert(ostream& out, const char* input) {
	xml_document doc;
	auto result = doc.load(input);
	if (!result) {
		cout << "\nXML content has syntax errors\n";
		cout << "Error description:\t" << result.description() << "\n";
		cout << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}


bool MusicXmlToHumdrumConverter::convert(ostream& out, xml_document& doc) {
	bool status = true; // for keeping track of problems in conversion process.

	vector<string> partids;            // list of part IDs
	map<string, xml_node> partinfo;    // mapping if IDs to score-part elements
	map<string, xml_node> partcontent; // mapping of IDs to part elements

	getPartInfo(partinfo, partids, doc);
	getPartContent(partcontent, partids, doc);
	vector<MxmlPart> partdata;
	partdata.resize(partids.size());
	fillPartData(partdata, partids, partinfo, partcontent);

	// for debugging:
	// printPartInfo(partids, partinfo, partcontent, partdata);

	HumGrid outdata;
	status &= stitchParts(outdata, partids, partinfo, partcontent, partdata);

	HumdrumFile outfile;
	outdata.transferTokens(outfile);
	for (int i=0; i<outfile.getLineCount(); i++) {
		outfile[i].createLineFromTokens();
	}
	out << outfile;

	return status;
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::setOptions --
//

void MusicXmlToHumdrumConverter::setOptions(int argc, char** argv) {
	m_options.process(argc, argv);
}


void MusicXmlToHumdrumConverter::setOptions(const vector<string>& argvlist) {
	int tempargc = (int)argvlist.size();
	char* tempargv[tempargc+1];
	tempargv[tempargc] = NULL;
	
	int i;
	for (i=0; i<tempargc; i++) {
		tempargv[i] = new char[argvlist[i].size() + 1];
		strcpy(tempargv[i], argvlist[i].c_str());
	}

	setOptions(tempargc, tempargv);

	for (i=0; i<tempargc; i++) {
		if (tempargv[i] != NULL) {
			delete [] tempargv[i];
		}
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::getOptionDefinitions -- Used to avoid
//     duplicating the definitions in the test main() function.
//

Options MusicXmlToHumdrumConverter::getOptionDefinitions(void) {
	return m_options;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// MusicXmlToHumdrumConverter::fillPartData --
//

bool MusicXmlToHumdrumConverter::fillPartData(vector<MxmlPart>& partdata,
		const vector<string>& partids, map<string, xml_node>& partinfo,
		map<string, xml_node>& partcontent) {

	bool output = true;
	for (int i=0; i<(int)partinfo.size(); i++) {
		partdata[i].setPartNumber(i+1);
		output &= fillPartData(partdata[i], partids[i], partinfo[partids[i]],
				partcontent[partids[i]]);
	}
	return output;
}


bool MusicXmlToHumdrumConverter::fillPartData(MxmlPart& partdata,
		const string& id, xml_node partdeclaration, xml_node partcontent) {
	auto measures = partcontent.select_nodes("./measure");
	for (int i=0; i<(int)measures.size(); i++) {
		partdata.addMeasure(measures[i].node());
	}
	return true;
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::printPartInfo -- Debug information.
//

void MusicXmlToHumdrumConverter::printPartInfo(vector<string>& partids,
		map<string, xml_node>& partinfo, map<string, xml_node>& partcontent,
		vector<MxmlPart>& partdata) {
	cout << "\nPart information in the file:" << endl;
	int maxmeasure = 0;
	for (int i=0; i<partids.size(); i++) {
		cout << "\tPART " << i+1 << " id = " << partids[i] << endl;
		cout << "\tMAXSTAFF " << partdata[i].getStaffCount() << endl;
		cout << "\t\tpart name:\t"
		     << getChildElementText(partinfo[partids[i]], "part-name") << endl;
		cout << "\t\tpart abbr:\t"
		     << getChildElementText(partinfo[partids[i]], "part-abbreviation")
		     << endl;
		auto node = partcontent[partids[i]];
		auto measures = node.select_nodes("./measure");
		cout << "\t\tMeasure count:\t" << measures.size() << endl;
		if (maxmeasure < (int)measures.size()) {
			maxmeasure = (int)measures.size();
		}
		cout << "\t\tTotal duration:\t" << partdata[i].getDuration() << endl;
	}

	MxmlMeasure* measure;
	for (int i=0; i<maxmeasure; i++) {
		cout << "m" << i+1 << "\t";
		for (int j=0; j<(int)partdata.size(); j++) {
			measure = partdata[j].getMeasure(i);
			if (measure) {
				cout << measure->getDuration();
			}
			if (j < (int)partdata.size() - 1) {
				cout << "\t";
			}
		}
		cout << endl;
	}
}



//////////////////////////////
//
// stitchParts -- Merge individual parts into a single score sequence.
//

bool MusicXmlToHumdrumConverter::stitchParts(HumGrid& outdata,
		vector<string>& partids, map<string, xml_node>& partinfo,
		map<string, xml_node>& partcontent, vector<MxmlPart>& partdata) {

	if (partdata.size() == 0) {
		return false;
	}

//	insertExclusiveInterpretationLine(outfile, partdata);

	int i;
	int measurecount = partdata[0].getMeasureCount();
	for (i=1; i<(int)partdata.size(); i++) {
		if (measurecount != partdata[i].getMeasureCount()) {
			cerr << "ERROR: cannot handle parts with different measure\n";
			cerr << "counts yet. Compare " << measurecount << " to ";
			cerr << partdata[i].getMeasureCount() << endl;
			exit(1);
		}
	}

	vector<int> partstaves(partdata.size(), 0);
	for (i=0; i<partstaves.size(); i++) {
		partstaves[i] = partdata[i].getStaffCount();
	}

	// vector<HumdrumLine*> measures;

	bool status = true;
	int m;
	for (m=0; m<partdata[0].getMeasureCount(); m++) {
		status &= insertMeasure(outdata, m, partdata, partstaves);
		// a hack for now:
		// insertSingleMeasure(outfile);
		// measures.push_back(&outfile[outfile.getLineCount()-1]);
	}

// Do this later, maybe in another class:
//	insertAllToken(outfile, partdata, "*-");
//	cleanupMeasures(outfile, measures);

	return status;
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::cleanupMeasures --
//     Also add barlines here (keeping track of the 
//     duration of each measure).
//

void MusicXmlToHumdrumConverter::cleanupMeasures(HumdrumFile& outfile,
		vector<HumdrumLine*> measures) {

   HumdrumToken* token;
	for (int i=0; i<outfile.getLineCount(); i++) {
		if (!outfile[i].isBarline()) {
			continue;
		}
		if (!outfile[i+1].isInterpretation()) {
			int fieldcount = outfile[i+1].getFieldCount();
			for (int j=1; j<fieldcount; j++) {
				token = new HumdrumToken("=");
				outfile[i].appendToken(token);
			}
		}
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::insertExclusiveInterpretationLine --
//

void MusicXmlToHumdrumConverter::insertExclusiveInterpretationLine(
		HumdrumFile& outfile, vector<MxmlPart>& partdata) {

	HumdrumLine* line = new HumdrumLine;
	HumdrumToken* token;

	int i, j;
	for (i=0; i<(int)partdata.size(); i++) {
		for (j=0; j<(int)partdata[i].getStaffCount(); j++) {
			token = new HumdrumToken("**kern");
			line->appendToken(token);
		}
		for (j=0; j<(int)partdata[i].getVerseCount(); j++) {
			token = new HumdrumToken("**kern");
			line->appendToken(token);
		}
	}
	outfile.appendLine(line);
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::insertSingleMeasure --
//

void MusicXmlToHumdrumConverter::insertSingleMeasure(HumdrumFile& outfile) {
	HumdrumLine* line = new HumdrumLine;
	HumdrumToken* token;
	token = new HumdrumToken("=");
	line->appendToken(token);
	line->createLineFromTokens();
	outfile.appendLine(line);
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::insertAllToken --
//

void MusicXmlToHumdrumConverter::insertAllToken(HumdrumFile& outfile,
		vector<MxmlPart>& partdata, const string& common) {

	HumdrumLine* line = new HumdrumLine;
	HumdrumToken* token;

	int i, j;
	for (i=0; i<(int)partdata.size(); i++) {
		for (j=0; j<(int)partdata[i].getStaffCount(); j++) {
			token = new HumdrumToken(common);
			line->appendToken(token);
		}
		for (j=0; j<(int)partdata[i].getVerseCount(); j++) {
			token = new HumdrumToken(common);
			line->appendToken(token);
		}
	}
	outfile.appendLine(line);
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::insertMeasure --
//

bool MusicXmlToHumdrumConverter::insertMeasure(HumGrid& outdata, int mnum,
		vector<MxmlPart>& partdata, vector<int> partstaves) {

	GridMeasure* gm = new GridMeasure;
	outdata.push_back(gm);

	vector<MxmlMeasure*> measuredata;
	vector<vector<SimultaneousEvents>* > sevents;
	int i;
	for (i=0; i<(int)partdata.size(); i++) {
		measuredata.push_back(partdata[i].getMeasure(mnum));
		sevents.push_back(measuredata.back()->getSortedEvents());
	}

	vector<HumNum> curtime(partdata.size());
	vector<int> curindex(partdata.size(), 0); // assuming data in a measure...
	HumNum nexttime = -1;
	for (i=0; i<(int)curtime.size(); i++) {
		curtime[i] = (*sevents[i])[curindex[i]].starttime;
		if (nexttime < 0) {
			nexttime = curtime[i];
		} else if (curtime[i] < nexttime) {
			nexttime = curtime[i];
		}
	}


	bool allend = false;
	vector<SimultaneousEvents*> nowevents;
	vector<int> nowparts;
	bool status = true;
	while (!allend) {
		nowevents.resize(0);
		nowparts.resize(0);
		allend = true;
		HumNum processtime = nexttime;
		nexttime = -1;
		for (i = (int)partdata.size()-1; i >= 0; i--) {
			if (curindex[i] >= (int)(*sevents[i]).size()) {
				continue;
			}
			if ((*sevents[i])[curindex[i]].starttime == processtime) {
				nowevents.push_back(&(*sevents[i])[curindex[i]]);
				nowparts.push_back(i);
				curindex[i]++;
			}
			if (curindex[i] < (int)(*sevents[i]).size()) {
				allend = false;
				if ((nexttime < 0) ||
						((*sevents[i])[curindex[i]].starttime < nexttime)) {
					nexttime = (*sevents[i])[curindex[i]].starttime;
				}
			}
		}
		status &= convertNowEvents(*outdata.back(),
		                         nowevents,
		                         nowparts,
		                         processtime,
		                         partdata,
		                         partstaves);
	}

	return status;
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::convertNowEvents --
//

bool MusicXmlToHumdrumConverter::convertNowEvents(
		GridMeasure& outdata,
		vector<SimultaneousEvents*>& nowevents,
		vector<int>& nowparts,
		HumNum nowtime,
		vector<MxmlPart>& partdata,
		vector<int>& partstaves) {

	if (nowevents.size() == 0) {
		// cout << "NOW EVENTS ARE EMPTY" << endl;
		return true;
	}

	appendZeroEvents(outdata, nowevents, nowtime, partdata);

	if (nowevents[0]->nonzerodur.size() == 0) {
		// no duration events (should be a terminal barline)
		// ignore and deal with in calling function.
		return true;
	}

	appendNonZeroEvents(outdata, nowevents, nowtime, partdata);

	return true;
}



/////////////////////////////
//
// MusicXmlToHumdrumConverter::appendNonZeroEvents --
//

void MusicXmlToHumdrumConverter::appendNonZeroEvents(
		GridMeasure& outdata,
		vector<SimultaneousEvents*>& nowevents,
		HumNum nowtime,
		vector<MxmlPart>& partdata) {

	GridSlice* slice = new GridSlice(nowtime, SliceType::Notes);
	outdata.push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)nowevents.size(); i++) {
		vector<MxmlEvent*>& events = nowevents[i]->nonzerodur;
		for (int j=0; j<(int)events.size(); j++) {
			addEvent(*slice, events[j]);
		}
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::addEvent --
//

void MusicXmlToHumdrumConverter::addEvent(GridSlice& slice,
		MxmlEvent* event) {

	int partindex;  // which part the event occurs in
	int staffindex; // which staff the event occurs in (need to fix)
	int voiceindex; // which voice the event occurs in (use for staff)

	partindex  = event->getPartIndex();
	staffindex = event->getStaffIndex();
	voiceindex = event->getVoiceIndex();

	string recip = event->getRecip();
	string pitch = event->getKernPitch();
	string other = event->getOtherNoteInfo();
	stringstream ss;
	ss << recip << pitch << other;

	HTp token = new HumdrumToken(ss.str());
	slice.at(partindex)->at(staffindex)->setTokenLayer(voiceindex, token,
		event->getDuration());
}



/////////////////////////////
//
// MusicXmlToHumdrumConverter::appendZeroEvents --
//

void MusicXmlToHumdrumConverter::appendZeroEvents(
		GridMeasure& outdata,
		vector<SimultaneousEvents*>& nowevents,
		HumNum nowtime,
		vector<MxmlPart>& partdata) {

	bool hasclef    = false;
	// bool haskeysig  = false;
	// bool hastimesig = false;

	vector<xml_node> clefs(partdata.size(), xml_node(NULL));
	int pindex = 0;
	xml_node child;

	for (int i=0; i<nowevents.size(); i++) {
		for (int j=0; j<nowevents[i]->zerodur.size(); j++) {
			xml_node element = nowevents[i]->zerodur[j]->getNode();
			if (nodeType(element, "attributes")) {
				child = element.first_child();
				while (child) {
					pindex = nowevents[i]->zerodur[j]->getPartIndex();
					if (nodeType(child, "clef") && !clefs[pindex]) {
						clefs[pindex] = child;
						hasclef = true;
					}
					child = child.next_sibling();
				}
			}
		}
	}

	if (hasclef) {
		addClefLine(outdata, clefs, partdata, nowtime);
	}

}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::addClefLine --
//

void MusicXmlToHumdrumConverter::addClefLine(GridMeasure& outdata, 
		vector<xml_node>& clefs, vector<MxmlPart>& partdata, HumNum nowtime) {

	GridSlice* slice = new GridSlice(nowtime, SliceType::Clefs);
	outdata.push_back(slice);
	slice->initializePartStaves(partdata);

	for (int i=0; i<(int)partdata.size(); i++) {
		if (clefs[i]) {
			insertPartClefs(clefs[i], *slice->at(i));
		}
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::insertPartClefs --
//

void MusicXmlToHumdrumConverter::insertPartClefs(xml_node clef, GridPart& part) {
	if (!clef) {
		// no clef for some reason.
		return;
	}

	HTp token;
	int staffnum = 0;
	while (clef) {
		clef = convertClefToHumdrum(clef, token, staffnum);
		part[staffnum]->setTokenLayer(0, token, 0);
	}
}



//////////////////////////////
//
//	MusicXmlToHumdrumConverter::convertClefToHumdrum --
//

xml_node MusicXmlToHumdrumConverter::convertClefToHumdrum(xml_node clef,
		HTp& token, int& staffindex) {

	if (!clef) {
		// no clef for some reason.
		return clef;
	}

	staffindex = 0;
	xml_attribute sn = clef.attribute("number");
	if (sn) {
		staffindex = atoi(sn.value()) - 1;
	}

	string sign;
	int line = 0;

	xml_node child = clef.first_child();
	while (child) {
		if (nodeType(child, "sign")) {
			sign = child.child_value();
		} else if (nodeType(child, "line")) {
			line = atoi(child.child_value());
		}
		child = child.next_sibling();
	}

	// Check for percussion clefs, etc., here.
	stringstream ss;
	ss << "*clef" << sign << line;
	token = new HumdrumToken(ss.str());

	clef = clef.next_sibling();
	if (!clef) {
		return clef;
	}
	if (nodeType(clef, "clef")) {
		return clef;
	} else {
		return xml_node(NULL);
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::nodeType -- return true if node type matches
//     string.
//

bool MusicXmlToHumdrumConverter::nodeType(xml_node node, const char* testname) {
	if (strcmp(node.name(), testname) == 0) {
		return true;
	} else {
		return false;
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::appendNullTokens --
//

void MusicXmlToHumdrumConverter::appendNullTokens(HumdrumLine* line,
		MxmlPart& part) {
	int i;
	int staffcount = part.getStaffCount();
	int versecount = part.getVerseCount();
	for (i=staffcount-1; i>=0; i--) {
		line->appendToken(".");
	}
	for (i=0; i<versecount; i++) {
		line->appendToken(".");
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::getPartContent -- Extract the part elements in
//     the file indexed by part ID.
//

bool MusicXmlToHumdrumConverter::getPartContent(
		map<string, xml_node>& partcontent,
		vector<string>& partids, xml_document& doc) {

	auto parts = doc.select_nodes("/score-partwise/part");
	int count = (int)parts.size();
	if (count != (int)partids.size()) {
		cerr << "Warning: part element count does not match part IDs count: "
		     << parts.size() << " compared to " << partids.size() << endl;
	}

	string partid;
	for (int i=0; i<(int)parts.size(); i++) {
		partid = getAttributeValue(parts[i], "id");
		if (partid.size() == 0) {
			cerr << "Warning: Part " << i << " has no ID" << endl;
		}
		auto status = partcontent.insert(make_pair(partid, parts[i].node()));
		if (status.second == false) {
			cerr << "Error: ID " << partids.back()
			     << " is duplicated and secondary part will be ignored" << endl;
		}
		if (find(partids.begin(), partids.end(), partid) == partids.end()) {
			cerr << "Error: Part ID " << partid
			     << " is not present in part-list element list" << endl;
			continue;
		}
	}

	if (partcontent.size() != partids.size()) {
		cerr << "Error: part-list count does not match part count "
		     << partcontent.size() << " compared to " << partids.size() << endl;
		return false;
	} else {
		return true;
	}
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::getPartInfo -- Extract a list of the part ids,
//    and a reverse mapping to the <score-part> element to which is refers.
//
//	   part-list structure:
//        <part-list>
//          <score-part id="P1"/>
//          <score-part id="P2"/>
//          etc.
//        </part-list>
//

bool MusicXmlToHumdrumConverter::getPartInfo(map<string, xml_node>& partinfo,
		vector<string>& partids, xml_document& doc) {
	auto scoreparts = doc.select_nodes("/score-partwise/part-list/score-part");
	partids.reserve(scoreparts.size());
	bool output = true;
	for (auto el : scoreparts) {
		partids.emplace_back(getAttributeValue(el.node(), "id"));
		auto status = partinfo.insert(make_pair(partids.back(), el.node()));
		if (status.second == false) {
			cerr << "Error: ID " << partids.back()
			     << " is duplicated and secondary part will be ignored" << endl;
		}
		output &= status.second;
		partinfo[partids.back()] = el.node();
	}
	return output;
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::getChildElementText -- Return the (first)
//    matching child element's text content.
//

string MusicXmlToHumdrumConverter::getChildElementText(xml_node root,
		const char* xpath) {
	return root.select_single_node(xpath).node().child_value();
}

string MusicXmlToHumdrumConverter::getChildElementText(xpath_node root,
		const char* xpath) {
	return root.node().select_single_node(xpath).node().child_value();
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::getAttributeValue -- For an xml_node, return
//     the value for the given attribute name.
//

string MusicXmlToHumdrumConverter::getAttributeValue(xml_node xnode,
		const string& target) {
	for (auto at = xnode.first_attribute(); at; at = at.next_attribute()) {
		if (target == at.name()) {
			return at.value();
		}
	}
	return "";
}


string MusicXmlToHumdrumConverter::getAttributeValue(xpath_node xnode,
		const string& target) {
	auto node = xnode.node();
	for (auto at = node.first_attribute(); at; at = at.next_attribute()) {
		if (target == at.name()) {
			return at.value();
		}
	}
	return "";
}



//////////////////////////////
//
// MusicXmlToHumdrumConverter::printAttributes -- Print list of all attributes
//     for an xml_node.
//

void MusicXmlToHumdrumConverter::printAttributes(xml_node node) {
	int counter = 1;
	for (auto at = node.first_attribute(); at; at = at.next_attribute()) {
		cout << "\tattribute " << counter++
		     << "\tname  = " << at.name()
		     << "\tvalue = " << at.value()
		     << endl;
	}
}

} // end namespace hum


