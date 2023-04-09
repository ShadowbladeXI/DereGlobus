/*
 * texts_map.h
 *
 *  Created on: Jan 2, 2023
 *      Author: Alex
 */

#ifndef TEXTS_MAP_H_
#define TEXTS_MAP_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <optional>

namespace dg{

//Class to store texts in different languages by their text id and the language id
class Texts_Map{
public:
	static constexpr std::string_view primaryLanguage = "german";
	static constexpr std::string_view primaryAlternativeLanguage = "english";

private:
	std::size_t primaryLanguage_index;
	std::size_t primaryAlternativeLanguage_index;

private:
	std::vector<std::string> languages;//All saved languages (as IDs)
	std::unordered_map<std::string, std::size_t> languages_map;//Save indexes of languages by their IDs
	std::unordered_map<std::string, std::vector<std::optional<std::string>>> texts_map;

public:
	Texts_Map(std::vector<std::string> languages)
		:languages(languages)
		,languages_map()
		,texts_map()
	{
		for(std::size_t language_index = 0; language_index<languages.size(); ++language_index){
			auto [insert_it, success] = languages_map.insert(std::make_pair(languages.at(language_index), language_index));
			if(!success){
				throw "Language " + std::string(insert_it->first) + " is duplicate!";//TODO: create dedicated exception class
			}
		}

		try{
			primaryLanguage_index = languages_map.at(std::string(primaryLanguage));
			primaryAlternativeLanguage_index = languages_map.at(std::string(primaryAlternativeLanguage));
		}catch(const std::out_of_range& exception){
			//One of the primary languages is missing
			throw exception;
		}
	}

	auto insert(std::pair<std::string, std::vector<std::optional<std::string>>> toInsert){
		if((toInsert.second).size() == languages.size()){
			return texts_map.insert(toInsert);
		}else{
			throw "Invalid number of translations provided!";//TODO: make dedicated exception
		}
	}

	const std::vector<std::optional<std::string>>& get_translations(const std::string& text_id) const{
		return texts_map.at(text_id);
	}

	//Get the text with the specific id in the specified language.
	//If it is not available in the specified language, it is returned in the highest priority language available
	//If it is available in no language, an empty optional is returned
	const std::optional<std::string>& get(const std::string& text_id, const std::string& language_id) const{
		const std::size_t& language_index = languages_map.at(language_id);
		const std::vector<std::optional<std::string>>& translations = texts_map.at(text_id);
		return get(translations, language_index);
	}

	//Get the text with the specific id in the specified language.
	//If it is not available in the specified language, it is returned in the highest priority language available
	//If it is available in no language, the id is returned
	const std::string& get_defaultToId(const std::string& text_id, const std::string& language_id) const{
		const std::optional<std::string>& text_opt = get(text_id, language_id);
		return defaultToId(text_id, text_opt);
	}

	//Get the text with the specific id in the specified language.
	//If it is available in no language, an empty optional is returned
	const std::optional<std::string>& get_noAlternativeLanguage(const std::string& text_id, const std::string& language_id) const{
		const std::size_t& language_index = languages_map.at(language_id);
		return texts_map.at(text_id).at(language_index);
	}

	//Get the text with the specific id in the specified language.
	//If it is available in no language, the id is returned
	const std::string& get_noAlternativeLanguage_defaultToId(const std::string& text_id, const std::string& language_id) const{
		const std::optional<std::string>& text_opt = get_noAlternativeLanguage(text_id, language_id);
		return defaultToId(text_id, text_opt);
	}

	//Get the text with the specific id in the default language.
	//If it is not available in the default language, it is returned in the highest priority language available
	//If it is available in no language, an empty optional is returned
	const std::optional<std::string>& get_inPrimaryLanguage(const std::string& text_id) const{
		const std::vector<std::optional<std::string>>& translations = texts_map.at(text_id);
		return get_inPrimaryLanguage(translations);
	}

	//Get the text with the specific id in the default language.
	//If it is not available in the default language, it is returned in the highest priority language available
	//If it is available in no language, the id is returned
	const std::string& get_inPrimaryLanguage_defaultToId(const std::string& text_id) const{
		const std::optional<std::string>& text_opt = get_inPrimaryLanguage(text_id);
		return defaultToId(text_id, text_opt);
	}

	//Get the text with the specific id in the default language.
	//If it is not available in the default language, an empty optional is returned
	const std::optional<std::string>& get_inPrimaryLanguage_noAlternativeLanguage(const std::string& text_id) const{
		return texts_map.at(text_id).at(primaryLanguage_index);
	}

	//Get the text with the specific id in the default language.
	//If it is not available in the default language, the id is returned
	const std::string& get_inPrimaryLanguage_noAlternativeLanguage_defaultToId(const std::string& text_id) const{
		const std::optional<std::string>& text_opt = get_inPrimaryLanguage_noAlternativeLanguage(text_id);
		return defaultToId(text_id, text_opt);
	}

	const std::vector<std::string>& get_languages() const{
		return languages;
	}

private:

	//Return requested language or highest priority available
	const std::optional<std::string>& get(const std::vector<std::optional<std::string>>& translations, std::size_t language_index) const{
		const std::optional<std::string>& text_opt = translations.at(primaryLanguage_index);
		if(text_opt.has_value()){
			return text_opt;
		}else{
			return get_inAlternativeLanguage(translations);
		}
	}

	//Return requested language or highest priority available
	const std::optional<std::string>& get_inPrimaryLanguage(const std::vector<std::optional<std::string>>& translations) const{
		return get(translations, primaryLanguage_index);
	}

	//Return id if supplied optional is empty
	const std::string& defaultToId(const std::string& text_id, const std::optional<std::string>& text_opt) const{
		if(text_opt.has_value()){
			return *text_opt;
		}else{
			return text_id;
		}
	}

	const std::optional<std::string>& get_inAlternativeLanguage(const std::vector<std::optional<std::string>>& translations) const{
		//Return primary alternative if available
		const std::optional<std::string>& text_opt = translations.at(primaryAlternativeLanguage_index);
		if(text_opt.has_value()){
			return text_opt;
		}

		//Return first available translation
		for(std::size_t language_index = 0; language_index < languages.size(); ++language_index){
			const std::optional<std::string>& text_opt = translations.at(language_index);
			if(text_opt.has_value()){
				return text_opt;
			}
		}

		//Return empty optional if nothing is available
		return text_opt;
	}

	/*const std::string& get_inAlternativeLanguage_defaultToId(const std::string& text_id, const std::vector<std::optional<std::string>>& translations){
		const std::optional<std::string>& text_opt = get_inAlternativeLanguage(translations);
		if(text_opt.has_value()){
			return *text_opt;
		}else{
			return text_id;
		}
	}*/

};

}

#endif /* TEXTS_MAP_H_ */
