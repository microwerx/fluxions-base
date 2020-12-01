#ifndef FLUXIONS_SIMPLE_MATERIAL_HPP
#define FLUXIONS_SIMPLE_MATERIAL_HPP

#include <cassert>
#include <fluxions_base_objects.hpp>
#include <fluxions_simple_loadable_resource.hpp>

namespace Fluxions {
	class SimpleMaterial : public SimpleLoadableResource {
	public:
		BaseMaterial base;

		bool hasMaps() const { return !maps_.empty(); }

		bool hasMap(const std::string& mapName) const { return maps_.count(mapName); }

		const string_string_map& maps() const { return maps_; }

		const std::string& getMapPath(const std::string& mapName) const {
			assert(maps_.count(mapName));
			return maps_.at(mapName);
		}

		// Adds a map to a material, where name = something like map_Kd and mapName is the actual name of the texture to load
		void addMap(const std::string& mapName, const std::string& mapPath) {
			maps_[mapName] = mapPath;
		}

	private:
		// key = mapName, value = mapPath
		string_string_map maps_;
	};
} // namespace Fluxions

#endif
