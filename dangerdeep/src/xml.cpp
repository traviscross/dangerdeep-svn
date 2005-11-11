// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "xml.h"
#include "tinyxml/tinyxml.h"


xml_elem xml_elem::child(const std::string& name)
{
	TiXmlElement* e = elem->FirstChildElement(name);
	if (!e) throw xml_elem_error(name, doc_name());
	return xml_elem(e);
}



xml_elem xml_elem::add_child(const std::string& name)
{
	TiXmlElement* e = new TiXmlElement(name);
	elem->LinkEndChild(e);
	return xml_elem(e);
}



std::string xml_elem::doc_name() const
{
	TiXmlDocument* doc = elem->GetDocument();
	// extra-Paranoia... should never happen
	if (!doc) throw xml_error(std::string("can't get document name for node ") + elem->Value(), "???");
	return doc->Value();
}



std::string xml_elem::attr(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return std::string(tmp);
	return std::string();
}



int xml_elem::attri(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return atoi(tmp);
	return 0;
}



unsigned xml_elem::attru(const std::string& name) const
{
	return unsigned(attri(name));
}



float xml_elem::attrf(const std::string& name) const
{
	const char* tmp = elem->Attribute(name);
	if (tmp) return atof(tmp);
	return 0.0f;
}



void xml_elem::set_attr(const std::string& name, const std::string& val)
{
	elem->SetAttribute(name, val);
}



void xml_elem::set_attr(const std::string& name, unsigned u)
{
	set_attr(name, int(u));
}



void xml_elem::set_attr(const std::string& name, int i)
{
	elem->SetAttribute(name, i);
}



void xml_elem::set_attr(const std::string& name, float f)
{
	char tmp[32];
	sprintf(tmp, "%f", f);
	set_attr(name, tmp);
}



std::string xml_elem::get_name() const
{
	return elem->Value();
}



xml_elem::iterator xml_elem::iterate(const std::string& childname) const
{
	return iterator(*this, elem->FirstChildElement(childname), true);
}



xml_elem::iterator xml_elem::iterate() const
{
	return iterator(*this, elem->FirstChildElement(), false);
}



xml_elem xml_elem::iterator::elem() const
{
	if (!e) throw xml_error("elem() on empty iterator", parent.doc_name());
	return xml_elem(e);
}



void xml_elem::iterator::next()
{
	if (!e) throw xml_error("next() on empty iterator", parent.doc_name());
	if (samename)
		e = e->NextSiblingElement(e->Value());
	else
		e = e->NextSiblingElement();
}



xml_doc::xml_doc(std::string fn)
	: doc(new TiXmlDocument(fn))
{
}



void xml_doc::load()
{
	if (!doc->LoadFile())
		throw xml_error("can't load", doc->Value());
}



void xml_doc::save()
{
	if (!doc->SaveFile())
		throw xml_error("can't save", doc->Value());
}



xml_elem xml_doc::child(const std::string& name)
{
	TiXmlElement* e = doc->FirstChildElement(name);
	if (!e) throw xml_elem_error(name, doc->Value());
	return xml_elem(e);
}



xml_elem xml_doc::add_child(const std::string& name)
{
	TiXmlElement* e = new TiXmlElement(name);
	doc->LinkEndChild(e);
	return xml_elem(e);
}



std::string xml_doc::get_filename() const
{
	return doc->Value();
}