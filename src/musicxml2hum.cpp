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

bool MusicXmlToHumdrumConverter::convertFile(ostream& out, const char* filename) {
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
	printPartInfo(partids, partinfo, partcontent, partdata);
	status &= stitchParts(out, partids, partinfo, partcontent, partdata);

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
// MusicXmlToHumdrumConverter::printPartInfo --
//

void MusicXmlToHumdrumConverter::printPartInfo(vector<string>& partids, map<string,
		xml_node>& partinfo, map<string, xml_node>& partcontent,
		vector<MxmlPart>& partdata) {
	cerr << "\nPart information in the file:" << endl;
	int maxmeasure = 0;
	for (int i=0; i<partids.size(); i++) {
		cerr << "\tPART " << i+1 << " id = " << partids[i] << endl;
		cerr << "\t\tpart name:\t"
		     << getChildElementText(partinfo[partids[i]], "part-name") << endl;
		cerr << "\t\tpart abbr:\t"
		     << getChildElementText(partinfo[partids[i]], "part-abbreviation")
		     << endl;
		auto node = partcontent[partids[i]];
		auto measures = node.select_nodes("./measure");
		cerr << "\t\tMeasure count:\t" << measures.size() << endl;
		if (maxmeasure < (int)measures.size()) {
			maxmeasure = (int)measures.size();
		}
		cerr << "\t\tTotal duration:\t" << partdata[i].getDuration() << endl;
	}

	MxmlMeasure* measure;
	for (int i=0; i<maxmeasure; i++) {
		cerr << "m" << i+1 << "\t";
		for (int j=0; j<(int)partdata.size(); j++) {
			measure = partdata[j].getMeasure(i);
			if (measure) {
				cerr << measure->getDuration();
			}
			if (j < (int)partdata.size() - 1) {
				cerr << "\t";
			}
		}
		cerr << endl;
	}
}


//////////////////////////////
//
// stitchParts -- Merge individual parts into a single score sequence.
//

bool MusicXmlToHumdrumConverter::stitchParts(ostream& out,
		vector<string>& partids, map<string, xml_node>& partinfo,
		map<string, xml_node>& partcontent, vector<MxmlPart>& partdata) {

	vector<int> partstaves(partdata.size(), 0);
	int i;
	for (i=0; i<partstaves.size(); i++) {
		partstaves[i] = partdata[i].getStaffCount();
	}

	cout << "\nPARTSTAVES:";
	for (i=0; i<partstaves.size(); i++) {
		cout << "\t" << partstaves[i];
	}
	cout << endl;

	return true;
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


