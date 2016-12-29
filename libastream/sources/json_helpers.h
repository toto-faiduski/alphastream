#pragma once

#include <iostream>
using namespace std;

#include "boost/optional.hpp"

#include "json_spirit_reader.h"
using namespace json_spirit;

ostream& JSON_Object_PrettyPrinter(ostream& os, const string& s, const mValue& value, const vector<string>& properties);
ostream& JSON_PrettyPrinter(ostream& os, const string& s, const mValue& value, bool show_header);

//#####################################################################################
// Get a JSON nullable value
//#####################################################################################
template<class _T, Value_type _VT>
boost::optional<_T> get_optional_value(mObject& object, const std::string& member)
{
	auto it = object.find(member);
	if (it == object.end())
		throw std::runtime_error("member not found");

	if (it->second.type() == null_type)
		return  boost::none;

	if (it->second.type() != _VT)
	{
		std::ostringstream os;
		os << "value type is " << it->second.type() << " not " << _VT;
		throw std::runtime_error(os.str());
	}

	return  it->second.get_value<_T>();
}

//#####################################################################################
// Get a JSON non-nullable value
//#####################################################################################
template<class _T, Value_type _VT>
_T get_mandatory_value(mObject& object, const std::string& member)
{
	auto it = object.find(member);
	if (it == object.end())
		throw std::runtime_error("member not found");

	if (it->second.type() != _VT)
	{
		std::ostringstream os;
		os << "value type is " << it->second.type() << " not " << _VT;
		throw std::runtime_error(os.str());
	}

	return  it->second.get_value<_T>();
}
