/*
 * dgkml_entity.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_ENTITY_H_
#define DEREGLOBUS_KML_DGKML_ENTITY_H_

#include "../external/rapidxml-1.13/rapidxml.hpp"

#include "kml_feature.h"

#include "dereGlobus.h"


// Feature_Type is the type of KML Feature the entity should be generated as.
template<typename KML_Feature_Type>
class DGKML_Entity : public KML_Feature_Type{

protected:
	std::optional<std::string> name_id;
	std::optional<std::string> description_id;

public:
	DGKML_Entity(const dg::Texts_Map& texts_map, dg::Texts_Map& texts_map_local, const dg::Entity& entity)
		: KML_Feature_Type()
	{
		name_id = entity.get_name_id();
		if(name_id.has_value()){
			texts_map_local.insert(std::make_pair(*name_id, texts_map.get_translations(*name_id)));
			KML_Feature_Type::set_name(texts_map_local.get_inPrimaryLanguage_defaultToId(*name_id));
		}else{
			KML_Feature_Type::set_name(entity.get_id() + ".name");
		}

		description_id = entity.get_description_id();
		if(description_id.has_value()){
			texts_map_local.insert(std::make_pair(*description_id, create_description(texts_map)));
			std::optional<std::string> description_dgkml = texts_map_local.get_inPrimaryLanguage(*description_id);

			if(description_dgkml.has_value()){
				KML_Feature_Type::set_description(*description_dgkml);
			}else{
				KML_Feature_Type::set_description("No description available in any language");//TODO: Test //TODO: replace by a constant
			}
		}
	}

	rapidxml::xml_node<kml::Ch_T>* create_xml(rapidxml::xml_document<kml::Ch_T>& doc) const override{
		auto* xml_node = KML_Feature_Type::create_xml(doc);

		auto* extData = doc.allocate_node(rapidxml::node_element, "ExtendedData");
		auto* dg_xmlns = doc.allocate_attribute("xmlns:dg", "http://www.dereglobus.orkenspalter.de");
		extData->append_attribute(dg_xmlns);
		auto* translationStructure = doc.allocate_node(rapidxml::node_element, "dg:translationStructure");
		extData->append_node(translationStructure);

		append_extendedData_content(doc, extData, translationStructure);

		xml_node->append_node(extData);

		return xml_node;
	}

protected:
	virtual void append_extendedData_content(rapidxml::xml_document<kml::Ch_T>& doc, rapidxml::xml_node<kml::Ch_T>* extData, rapidxml::xml_node<kml::Ch_T>* translationStructure) const{
		if(name_id.has_value()){
			auto* name_node = doc.allocate_node(rapidxml::node_element, "name");
			translationStructure->append_node(name_node);
			auto* translationMarker = doc.allocate_node(rapidxml::node_element, "dg:translationMarker");
			auto* translationName = doc.allocate_attribute("translationName", name_id->c_str());
			translationMarker->append_attribute(translationName);
			name_node->append_node(translationMarker);
		}
	}

	//create description for all languages
	virtual std::vector<std::optional<std::string>> create_description(const dg::Texts_Map& texts_map) const {
		auto translations = std::vector<std::optional<std::string>>();
		translations.reserve(texts_map.get_languages().size());

		for(const std::string& language : texts_map.get_languages()){
			translations.push_back(create_description(texts_map, language));
		}

		return translations;
	}

	//create description for a single language
	virtual std::optional<std::string> create_description(const dg::Texts_Map& texts_map, std::string language) const {
		if(description_id.has_value()){
			const std::optional<std::string>& description_dg = texts_map.get_noAlternativeLanguage(*description_id, language);
			if(description_dg.has_value()){
				return create_description(texts_map, language, *description_dg);
			}else{
				return std::nullopt;
			}
		}else{
			return std::nullopt;
		}
	}

	//create description for a single language based on the description from the DereGlobus database
	//By dfault this is only called if there is a description of that language
	virtual std::optional<std::string> create_description(const dg::Texts_Map& texts_map, std::string language, std::string description_dg) const{
		return std::make_optional(description_dg);
	}
};



#endif /* DEREGLOBUS_KML_DGKML_ENTITY_H_ */
