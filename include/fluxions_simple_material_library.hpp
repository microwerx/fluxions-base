#ifndef FLUXIONS_SIMPLE_MATERIAL_LIBRARY_HPP
#define FLUXIONS_SIMPLE_MATERIAL_LIBRARY_HPP

#include <fluxions_base.hpp>
#include <fluxions_simple_loadable_resource.hpp>
#include <fluxions_simple_material.hpp>

namespace Fluxions {
	class SimpleMaterialLibrary : public SimpleLoadableResource {
	public:
		// texture maps [k, v] = [map name, path to image]
		string_string_map maps;

		// material libraries [k, v] = [mtllib name, path to MTL]
		string_string_map mtllibs;

		// materials
		std::vector<SimpleMaterial> mtls;

		size_t sizeInBytes() const override;

		static constexpr int WRITE_MATERIALS = 0x0001;
		static constexpr int WRITE_MAPS = 0x0002;
		static constexpr int WRITE_SPECULAR = 0x0004;

		void clear();
		bool loadMTLs(const string_string_map& mtlsToLoad);
		bool loadMTL(const std::string& path);
		bool saveMTL(const std::string& path) const;
		bool saveXml(const std::string& path, int flags) const;

		int getMaterialIndex(const std::string& materialName) const;

	private:
		SimpleMaterial* set_mtl(std::string&& name);
		// Looks for map location, modifies mapname to be user friendly
		bool readMap(const std::string& mapname, std::string& key, const std::string& pathToMap);
		std::ostream& printMap(std::ostream& ostr, const std::string& maptype, const std::string& mapname) const;
	};
}

#endif
