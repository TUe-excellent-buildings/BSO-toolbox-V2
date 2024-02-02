#ifndef SD_TRIANGLE_HPP
#define SD_TRIANGLE_HPP

#include <bso/structural_design/component/geometry.hpp>
#include <bso/utilities/geometry/triangle.hpp>

namespace bso { namespace structural_design { namespace component {
	
	class triangle : public bso::utilities::geometry::triangle,
												public geometry
	{
	private:

	public:
		template <typename TRIANGLE_INITIALIZER>
		triangle(const TRIANGLE_INITIALIZER& l);
		triangle(std::initializer_list<bso::utilities::geometry::vertex>&& l);
		~triangle();
		
		void addStructure(const structure& s);
		void mesh(const unsigned int& n1, std::vector<point*>& pointStore);
	};
	
} // namespace component
} // namespace structural_design
} // namespace bso

#include <bso/structural_design/component/triangle.cpp>

#endif // SD_TRIANGLE_HPP