/*
 * op_manager.cc
 *
 */

#include "bbque/rtlib/monitors/op_manager.h"

using namespace std;

OP_Manager::~OP_Manager() {
	OperatingPointsList::iterator it;
	for (it = operatingPoints.begin(); it != operatingPoints.end(); ++it)
		delete *it;
	operatingPoints.clear();
}

void OP_Manager::parseDoc(string documentName) {
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(documentName.c_str());

	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		fprintf(stderr,"Empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "points")) {
		fprintf(stderr,"Wrong document's type, root node != points\n");
		xmlFreeDoc(doc);
		return;
	}

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"point"))) {
			OperatingPoint* newPoint = new OperatingPoint();
			parsePoint (doc, cur, newPoint);
			operatingPoints.push_back(newPoint);
		}

		cur = cur->next;
	}

	xmlFreeDoc(doc);
	return;

}

void OP_Manager::parsePoint (xmlDocPtr doc,
			     xmlNodePtr cur,
			     OperatingPoint *operatingPoint) {

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameters"))) {
			parseParameters(doc,cur, operatingPoint);
		}
		else if ((!xmlStrcmp(cur->name,
				    (const xmlChar *)"system_metrics"))) {
			parseMetrics(doc,cur, operatingPoint);
		}
		cur = cur->next;
	}
	return;
}

void OP_Manager::parseMetrics(xmlDocPtr doc,
			      xmlNodePtr cur,
			      OperatingPoint *operatingPoint) {

	xmlChar *name;
	xmlChar *value;

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"system_metric"))) {
			name = xmlGetProp(cur, BAD_CAST "name");
			value = xmlGetProp(cur, BAD_CAST "value");

			operatingPoint->metrics[string(
				reinterpret_cast<const char *>(name))] =
				atof(reinterpret_cast<const char *>(value));

			xmlFree(name);
			xmlFree(value);

		}
		cur = cur->next;
	}
	return;
}

void OP_Manager::parseParameters(xmlDocPtr doc,
				 xmlNodePtr cur,
				 OperatingPoint *operatingPoint) {
	xmlChar *name;
	xmlChar *value;

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter"))) {
			name = xmlGetProp(cur, BAD_CAST "name");
			value = xmlGetProp(cur, BAD_CAST "value");

			operatingPoint->parameters[string(
				reinterpret_cast<const char *>(name))] =
				atof(reinterpret_cast<const char *>(value));

			xmlFree(name);
			xmlFree(value);

		}
		cur = cur->next;
	}
	return;
}

bool OP_Manager::operatingPointsComparator::operator()(OperatingPoint *op1,
						       OperatingPoint *op2) {
	for (unsigned int i=0; i < metricsPriorities.size(); ++i){
		double val1 = op1->metrics[metricsPriorities.at(i).metricName];
		double val2 = op2->metrics[metricsPriorities.at(i).metricName];
		if (val1 == val2)
			continue;
		return metricsPriorities.at(i).comparisonFunction(val1,val2);
	}
	return false;
}

bool OP_Manager::getCurrentOP(OperatingPoint &op) {
	op = *(operatingPoints[vectorId]);
	return true;
}

bool OP_Manager::getLowerOP(OperatingPoint &op) {
	if (vectorId >= (operatingPoints.size()-1))
		return false;

	vectorId++;
	op = *(operatingPoints[vectorId]);
	return true;
}

bool OP_Manager::getHigherOP(OperatingPoint &op) {
	if (vectorId == 0)
		return false;

	vectorId--;
	op = *(operatingPoints[vectorId]);
	return true;
}

bool OP_Manager::isValidOP(OperatingPoint *op,
			   const OP_FilterList &opFilters) const {
	bool result = true;
	OP_FilterList::const_iterator filter = opFilters.begin();;
	std::map<std::string, double>::iterator mappedValue;

	while (result && filter!=opFilters.end()) {
		mappedValue = op->metrics.find(filter->name);
		if (mappedValue == op->metrics.end())
			continue;
		result = (filter->cFunction(mappedValue->second, filter->value));
		++filter;
	}
	return result;
}


bool OP_Manager::getCurrentOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	if (isValidOP(operatingPoints[vectorId], opFilters)){
		op = *(operatingPoints[vectorId]);
		return true;
	}
	return getLowerOP(op,opFilters);
}

bool OP_Manager::getLowerOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	for (unsigned int id = vectorId + 1;id < operatingPoints.size(); ++id) {
		if (isValidOP(operatingPoints[id],opFilters)){
			vectorId = id;
			op = *(operatingPoints[id]);
			return true;
		}
	}
	return false;
}

bool OP_Manager::getHigherOP(OperatingPoint &op, const OP_FilterList &opFilters) {
	for (int id = vectorId-1;id > 0; --id){
		if (isValidOP(operatingPoints[id],opFilters)){
			vectorId = id;
			op = *(operatingPoints[id]);
			return true;
		}
	}
	return false;
}

void OP_Manager::setPolicy(PrioritiesList &orderingStrategy) {
	sort(operatingPoints.begin(),
	     operatingPoints.end(),
	     operatingPointsComparator(orderingStrategy));

	vectorId = 0;
}
