#include "json_helpers.h"

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/optional/optional_io.hpp>

//#####################################################################################
// print JSON property
//#####################################################################################
namespace std {
	ostream& operator<<(ostream& os, const mValue& v)
	{
		switch (v.type())
		{
		case str_type: os << v.get_str(); break;
		case bool_type: os << v.get_bool(); break;
		case int_type: os << v.get_int(); break;
		case real_type: os << v.get_real(); break;
		}
		return os;
	};
}

//#####################################################################################
// Pretty printer for a JSON aobject
//#####################################################################################
ostream& JSON_Object_PrettyPrinter(ostream& os, const string& s, const mValue& value, const vector<string>& properties)
{
	// on doit avoir un objet
	if (value.type() != obj_type)
	{
		os << "not an object";
		return os;
	}

	try
	{
		boost::format fmt(s);
		mObject object = value.get_obj();

		// On parcourt toutes les propriétés trouvées
		for (auto itProp = properties.begin(); itProp != properties.end(); itProp++)
		{
			// On les cherche dans l'objet
			auto it = object.find(*itProp);

			// On l'ajoute au format
			if (it == object.end() || it->second.type() == null_type)
				fmt % boost::none;
			else
				fmt % it->second;
		}

		os << fmt;
	}
	catch (std::runtime_error ex)
	{
		return os;
	}
	return os;
}

//#####################################################################################
// regex formatter
//#####################################################################################
struct PropertyFormatter {
	std::string operator()(boost::smatch match)
	{
		//Save property name : '%|*name*|' => 'name'
		properties.push_back(string(match[1], 3, match[1].length() - 5));
		return "%||";
	};
	vector<string> properties;
};

//#####################################################################################
// Pretty printer for a JSON object/array
//#####################################################################################
ostream& JSON_PrettyPrinter(ostream& os, const string& s, const mValue& value, bool show_header)
{
	// On remplace toutes les arguments "%|*......*|" par "%||" tout en mémorisant les noms des propriétés
	PropertyFormatter formatter;
	const boost::regex re("(%[|][*]\\w+[*][|])");
	string replaced = boost::regex_replace(s, re, boost::ref(formatter));

	// Affichage du header de tableau
	if (show_header)
	{
		boost::format fmt(replaced);
		// On parcourt toutes les propriétés trouvées
		for (auto itProp = formatter.properties.begin(); itProp != formatter.properties.end(); itProp++)
		{
			fmt % *itProp;
		}
		os << fmt << endl;
	}

	// Pour les array 
	if (value.type() == array_type)
	{
		// On affiche tous les éléments
		auto array = value.get_array();
		for (auto itJob = array.begin(); itJob != array.end(); itJob++)
		{
			JSON_Object_PrettyPrinter(os, replaced, *itJob, formatter.properties) << endl;
		}
	}
	// Pour les objets
	else if (value.type() == obj_type)
	{
		// On affiche juste l'objet !
		JSON_Object_PrettyPrinter(os, replaced, value, formatter.properties) << endl;
	}
	else
	{
		os << "not an object or array" << endl;
	}

	return os;
}

