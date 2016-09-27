//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  6 10:53:40 CEST 2016
// Last Modified: Sun Sep 18 13:33:18 PDT 2016
// Filename:      musicxml2hum.h
// URL:           https://github.com/craigsapp/musicxml2hum/blob/master/include/musicxml2hum.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Inteface to convert a MusicXml file into a Humdrum file.
//

#ifndef _MUSICXML2HUM_H
#define _MUSICXML2HUM_H

#define _USE_HUMLIB_OPTIONS_
#include "humlib.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include "MxmlPart.h"
#include "MxmlMeasure.h"
#include "MxmlEvent.h"

using namespace std;
using namespace pugi;

namespace hum {



class MusicXmlToHumdrumConverter {
	public:
		MusicXmlToHumdrumConverter(void);
		~MusicXmlToHumdrumConverter() {}

		bool    convertFile          (ostream& out, const char* filename);
		bool    convert              (ostream& out, xml_document& infile);
		bool    convert              (ostream& out, const char* input);
		bool    convert              (ostream& out, istream& input);
		void    setOptions           (int argc, char** argv);
		void    setOptions           (const vector<string>& argvlist);
		Options getOptionDefinitions (void);

	protected:
		string getChildElementText  (xml_node root, const char* xpath);
		string getChildElementText  (xpath_node root, const char* xpath);
		string getAttributeValue    (xml_node xnode, const string& target);
		string getAttributeValue    (xpath_node xnode, const string& target);
		void   printAttributes      (xml_node node);
		bool   getPartInfo          (map<string, xml_node>& partinfo,
		                             vector<string>& partids, xml_document& doc);
		bool   getPartContent       (map<string, xml_node>& partcontent,
		                             vector<string>& partids, xml_document& doc);
		void   printPartInfo        (vector<string>& partids,
		                             map<string, xml_node>& partinfo,
		                             map<string, xml_node>& partcontent,
		                             vector<MxmlPart>& partdata);
		bool   fillPartData         (vector<MxmlPart>& partdata,
		                             const vector<string>& partids,
		                             map<string, xml_node>& partinfo,
		                             map<string, xml_node>& partcontent);
		bool   fillPartData         (MxmlPart& partdata, const string& id,
		                             xml_node partdeclaration,
		                             xml_node partcontent);

		bool convert          (ostream& out);
		bool convertPart      (ostream& out, const string& partname,
		                       int partindex);

	private:
		Options m_options;

};


}  // end of namespace hum

#endif /* _MUSICXML2HUM_H */



