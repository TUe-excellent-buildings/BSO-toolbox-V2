#ifndef BSO_DL_SPACE_PROPERTIES_HPP
#define BSO_DL_SPACE_PROPERTIES_HPP

#include <string>
#include <tuple>
#include <vector>

namespace bso { namespace daylight { namespace properties {

class space_properties
{
private:
	std::string mID;
	bool mIsExternal = false;
	bool mIsOpen = false;
	
public:
	space_properties(const std::string& ID = "", const bool isExternal = false, const bool isOpen = false);
	~space_properties();
	
	const std::string& getID() const {return mID;}
	bool isExternal() const {return mIsExternal;}
	bool isOpen() const {return mIsOpen;}
};

} // namespace properties
} // namespace daylight
} // namespace bso

#include <bso/daylight/properties/space_properties.cpp>

#endif // BSO_DL_SPACE_PROPERTIES_HPP