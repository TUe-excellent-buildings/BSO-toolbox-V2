#ifndef SD_LOAD_COMPONENT_HPP
#define SD_LOAD_COMPONENT_HPP

namespace bso { namespace structural_design { namespace component {
	
	class load_case
	{
	private:
		std::string mDescription = "";
	public:
		load_case(std::string description = "");
		~load_case();
		
		const std::string& description() const;
		
		bool operator == (const load_case& rhs) const;
		bool operator > (const load_case& rhs) const;
		bool operator >= (const load_case& rhs) const;
		bool operator < (const load_case& rhs) const;
		bool operator <= (const load_case& rhs) const;
		friend std::ostream& operator << (std::ostream& stream, const load_case& lc);
	};
	
	class load
	{
	private:
		load_case mLoadCase; //Describes to which load case this load components bellongs to
		double mMagnitude; //Described the magnitude of one of the component (x,y,z) of a load of a load case defined in the input file. This value may be minus and describes that the load is against the positive assumed directions (1,0,0)/(0,1,0)/(0,0,1)
		unsigned int mDOF; // Direction Of Force stored in either x/y/z defined as integer 0/1/2 corosponding to (1,0,0)/(0,1,0)/(0,0,1) in the global coördinate system.
	public:
		load(load_case lc, const double& magnitude, const unsigned int& DOF);
		~load();
		
		const load_case& loadCase() const {return mLoadCase;};
		const double& magnitude() const {return mMagnitude;};
		const unsigned int& DOF() const {return mDOF;};
		
		load& operator += (const double& rhs);
		load& operator -= (const double& rhs);
		load& operator *= (const double& rhs);
		load& operator /= (const double& rhs);
		
	};
	
	load operator + (load lhs, const double& rhs);
	load operator - (load lhs, const double& rhs);
	load operator * (load lhs, const double& rhs);
	load operator / (load lhs, const double& rhs);
	
} // namespace component
namespace element {
	typedef component::load_case load_case;
	typedef component::load load;
} // namespace element
} // namespace structural_design
} // namespace bso

#include <bso/structural_design/component/load.cpp>

#endif // SD_LOAD_COMPONENT_HPP