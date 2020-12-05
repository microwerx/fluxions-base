#include "fluxions_base_pch.hpp"
#include <fluxions_file_system.hpp>
#include <fluxions_simple_material.hpp>
#include <fluxions_simple_material_library.hpp>

namespace Fluxions {
	size_t SimpleMaterialLibrary::sizeInBytes() const {
		size_t sizeOfMaterials = 0;
		for (const auto& m : mtls) {
			sizeOfMaterials += m.sizeInBytes();
		}
		return Fluxions::sizeInBytes(maps) + Fluxions::sizeInBytes(mtllibs) + sizeOfMaterials;
	}

	void SimpleMaterialLibrary::clear() {
		maps.clear();
		mtllibs.clear();
		mtls.clear();
	}

	bool SimpleMaterialLibrary::loadMTLs(const string_string_map& mtlsToLoad) {
		bool result = true;
		for (auto& [name, path] : mtlsToLoad) {
			if (!loadMTL(path)) result = false;
		}
		return result;
	}

	bool SimpleMaterialLibrary::loadMTL(const std::string& filename) {
		FilePathInfo fpi(filename);
		std::string name = fpi.filename();
		if (mtllibs.count(name)) {
			return true;
		}

		HFLOGINFO("'%s' ... loading '%s'", name.c_str(), fpi.shortestPathC());

		std::string pathToMTL = fpi.shortestPath();
		mtllibs[name] = pathToMTL;

		std::vector<std::string> lines;
		ReadLines(pathToMTL, lines, false);

		// This helps us find the path to the maps in the current directory
		FilePathFinder pathFinder;
		pathFinder.push(fpi.parentPath());

		SimpleMaterial* curmtl = nullptr;
		for (auto& line : lines) {
			std::string str;
			std::stringstream istr(line);

			istr >> str;
			if (str == "newmtl") {
				curmtl = set_mtl(toloweridentifier(ReadString(istr)));
				HFLOGINFO("'%s' ... newmtl '%s'", name.c_str(), curmtl->name_cstr());
			}
			else if (curmtl == nullptr) {
			} // do nothing from now on because we're null
			else if (str.empty()) {
			} // do nothing if there is an empty string
			else if (str.size() >= 3 && str[0] == '#' && str[1] != 'P' && str[2] != 'B') {
			} // do nothing because it is a comment, not a #PB...
			else if (str.size() < 3 && str[0] == '#') {
			} // do nothing because it's a 2 character short comment
			else if (str == "Kd") {
				curmtl->base.Kd = ReadColor3f(istr);
			}
			else if (str == "Ks") {
				curmtl->base.Ks = ReadColor3f(istr);
			}
			else if (str == "Ke") {
				curmtl->base.Ke = ReadColor3f(istr);
			}
			else if (str == "ior") {
				curmtl->base.Kior.reset(ReadFloat(istr));
			}
			else if (str == "Ns") {
				curmtl->base.Ksroughness.r = shininessToAlpha(ReadFloat(istr));
			}
			else if (str == "#PBKdm" || str == "Kdm") {
				curmtl->base.Kdroughness.r = ReadFloat(istr);
			}
			else if (str == "#PBKdfuzzydusty" || str == "Kdfuzzydusty") {
				curmtl->base.Kdroughness.g = ReadFloat(istr);
			}
			else if (str == "#PBKdsubsurface" || str == "Kdsubsurface") {
				curmtl->base.Kdroughness.b = ReadFloat(istr);
			}
			else if (str == "#PBKsm" || str == "Ksm") {
				curmtl->base.Ksroughness.r = ReadFloat(istr);
			}
			else if (str == "#PBKsGGXgamma" || str == "KsGGXgamma") {
				curmtl->base.Ksroughness.g = ReadFloat(istr);
			}
			else if (str == "#PBKsaniso" || str == "Ksaniso" || str == "aniso") {
				curmtl->base.Ksroughness.b = ReadFloat(istr);
			}
			else if (str == "#PBKsior" || str == "Ksior") {
				curmtl->base.Kior = ReadColor3f(istr);
			}
			else if (str == "#PBKdior" || str == "Kdior") {
				curmtl->base.Kior.a = ReadFloat(istr);
			}
			else if (str == "#PBKsmetallic" || str == "Ksmetallic" || str == "metallic" || str == "Pm") {
				curmtl->base.KmetallicSpecular.r = ReadFloat(istr);
			}
			else if (str == "#PBKsspecular" || str == "Ksspecular" || str == "specular") {
				curmtl->base.KmetallicSpecular.g = ReadFloat(istr);
			}
			else if (str == "#PBKsspeculartint" || str == "Ksspeculartint" || str == "speculartint") {
				curmtl->base.KmetallicSpecular.b = ReadFloat(istr);
			}
			else if (str == "#PBKsanisor" || str == "Ksanisor" || str == "anisor") {
				curmtl->base.KmetallicSpecular.a = ReadFloat(istr);
			}
			else if (str == "Pc") {
				curmtl->base.KclearcoatSheen.r = ReadFloat(istr);
			}
			else if (str == "Pcr") {
				curmtl->base.KclearcoatSheen.g = ReadFloat(istr);
			}
			else if (str == "sheen") {
				curmtl->base.KclearcoatSheen.b = ReadFloat(istr);
			}
			else if (str == "sheentint") {
				curmtl->base.KclearcoatSheen.a = ReadFloat(istr);
			}
			else if (str.substr(0, 4) == "map_") {
				std::string mapName = str;
				std::string pathToMap = ReadString(istr);
				std::string shortestPathToMap = pathFinder.findShortestPath(pathToMap);
				if (shortestPathToMap.empty()) {
					HFLOGERROR("'%s' ... map '%s' not found", name.c_str(), shortestPathToMap.c_str());
				}
				else {
					curmtl->addMap(mapName, shortestPathToMap);
					std::string key = tolower(pathFinder.fpi().filename());
					readMap(mapName, key, shortestPathToMap);

					if (str == "map_Kd") {
						curmtl->base.Kd.a = 1.0f;
					}
					else if (str == "map_Ks") {
						curmtl->base.Ks.a = 1.0f;
					}
					else if (str == "map_Ke") {
						curmtl->base.Ke.a = 1.0f;
					}
					else if (str == "map_Kdpbr") {
						curmtl->base.Kdroughness.a = 1.0f;
					}
					else if (str == "map_Kspbr") {
						curmtl->base.Ksroughness.a = 1.0f;
					}
				}
			}
			else if (!str.empty()) {
				//HFLOGWARN("property %s not officially supported", str.c_str());
			}
		}

		return true;
	}

	bool SimpleMaterialLibrary::saveMTL(const std::string& filename) const {
		FilePathInfo fpi(filename);
		const char* endl = "\n    ";
		std::ofstream fout(filename);
		for (const auto& mtl : mtls) {
			WriteLabel(fout, "newmtl");
			WriteString(fout, mtl.name()) << endl;
			WriteLabel(fout, "Kd"); WriteColor3f(fout, mtl.base.Kd.rgb()) << endl;
			WriteLabel(fout, "Ks"); WriteColor3f(fout, mtl.base.Ks.rgb()) << endl;
			WriteLabel(fout, "Ke"); WriteColor3f(fout, mtl.base.Ke.rgb()) << endl;
			WriteLabel(fout, "Ns"); WriteFloat(fout, alphaToShininess(mtl.base.Ksroughness.r)) << endl;
			WriteLabel(fout, "ior"); WriteFloat(fout, mtl.base.Kior.r) << endl;
			WriteLabel(fout, "Ksior"); WriteColor3f(fout, mtl.base.Kior.rgb()) << endl;
			WriteLabel(fout, "Kdior"); WriteColor3f(fout, mtl.base.Kior.a) << endl;
			WriteLabel(fout, "Kdm"); WriteColor3f(fout, mtl.base.Kdroughness.r) << endl;
			WriteLabel(fout, "Kdfuzzydusty"); WriteColor3f(fout, mtl.base.Kdroughness.g) << endl;
			WriteLabel(fout, "Kdsubsurface"); WriteColor3f(fout, mtl.base.Kdroughness.b) << endl;
			WriteLabel(fout, "Ksm"); WriteColor3f(fout, mtl.base.Ksroughness.r) << endl;
			WriteLabel(fout, "KsGGXgamma"); WriteColor3f(fout, mtl.base.Ksroughness.g) << endl;
			WriteLabel(fout, "aniso"); WriteColor3f(fout, mtl.base.Ksroughness.b) << endl;
			WriteLabel(fout, "anisor"); WriteColor3f(fout, mtl.base.KmetallicSpecular.a) << endl;
			WriteLabel(fout, "Pm"); WriteFloat(fout, alphaToShininess(mtl.base.KmetallicSpecular.r)) << endl;
			WriteLabel(fout, "specular"); WriteFloat(fout, alphaToShininess(mtl.base.KmetallicSpecular.g)) << endl;
			WriteLabel(fout, "speculartint"); WriteFloat(fout, alphaToShininess(mtl.base.KmetallicSpecular.b)) << endl;
			WriteLabel(fout, "Pc"); WriteFloat(fout, alphaToShininess(mtl.base.KclearcoatSheen.r)) << endl;
			WriteLabel(fout, "Pcr"); WriteFloat(fout, alphaToShininess(mtl.base.KclearcoatSheen.g)) << endl;
			WriteLabel(fout, "sheen"); WriteFloat(fout, alphaToShininess(mtl.base.KclearcoatSheen.b)) << endl;
			WriteLabel(fout, "sheentint"); WriteFloat(fout, alphaToShininess(mtl.base.KclearcoatSheen.a)) << endl;
			for (auto& map : mtl.maps()) {
				printMap(fout, map.first, map.second) << endl;
			}
		}
		return true;
	}

	bool SimpleMaterialLibrary::saveXml(const std::string& path, int flags) const {
		FilePathInfo fpi(path);
		const char* endl = "\n    ";
		std::ofstream mtlxml_fout(path);
		XmlBeginTag(mtlxml_fout, "mtlLib") << endl;
		if (flags & WRITE_MATERIALS) {
			bool enableSpecular = (flags & WRITE_SPECULAR);
			int materialNumber = 0;
			for (const auto& mtl : mtls) {
				XmlComment(mtlxml_fout, "Material" + std::to_string(materialNumber++));

				XmlBeginTag(mtlxml_fout, "materialDefinition", "name", mtl.name(), 1) << "\n";
				XmlBeginTag(mtlxml_fout, "material", "Native", 2) << "\n";

				bool isMetal = enableSpecular && mtl.base.KmetallicSpecular.r > 0.0f;
				if (mtl.hasMap("map_Kd")) {
					XmlBeginTag(mtlxml_fout, "diffuse", 3) << "\n";
					XmlBeginTag(mtlxml_fout, "map", "Reference", 4);
					std::string map_name = "assets/" + mtl.getMapPath("map_Kd");
					toidentifier(map_name);
					mtlxml_fout << map_name;
					XmlEndTag(mtlxml_fout, "map") << "\n";
					XmlEndTag(mtlxml_fout, "diffuse", 3) << "\n";
				}
				else if (isMetal)
					XmlColor3f(mtlxml_fout, "diffuse", Color3f(), 3) << "\n";
				else
					XmlColor3f(mtlxml_fout, "diffuse", mtl.base.Kd, 3) << "\n";

				if (enableSpecular) {
					XmlBeginTag(mtlxml_fout, "reflect", 3) << "\n";
					XmlColor3f(mtlxml_fout, "color", mtl.base.Ks, 4) << "\n";

					if (isMetal) {
						XmlFloat(mtlxml_fout, "ior", 999, 4) << "\n";
					}
					else {
						XmlFloat(mtlxml_fout, "ior", mtl.base.Kior.r, 4) << "\n";
					}
					// Corona uses glossiness which is the reverse of roughness
					// A surface is smooth if roughness is 0 or glossiness is 1
					// A surface is rough if roughness is 1 or glossiness is 0
					if (mtl.base.Ksroughness.r < 0.0f)
						XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f + mtl.base.Ksroughness.r, 0.0f, 1.0f), 4) << "\n";
					else
						XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f - mtl.base.Ksroughness.r, 0.0f, 1.0f), 4) << "\n";
					XmlEndTag(mtlxml_fout, "reflect", 3) << "\n";
				}

				XmlEndTag(mtlxml_fout, "material", 2) << "\n";
				XmlEndTag(mtlxml_fout, "materialDefinition", 1) << "\n\n";
			}
		}

		if (flags & WRITE_MAPS) {
			XmlComment(mtlxml_fout, "Texture Maps", 1) << "\n\n";
			for (auto& [mapName, imagePath] : maps) {
				std::string map_name = "assets/" + imagePath;
				toidentifier(map_name);
				FilePathInfo mapfpi(imagePath);
				std::string map_path = "assets/" + mapfpi.filename();

				XmlCoronaMapTexture(mtlxml_fout, "mapDefinition", map_name, map_path, 1, 2.2f) << "\n\n";
			}
		}

		XmlEndTag(mtlxml_fout, "mtlLib");
		return true;
	}

	int SimpleMaterialLibrary::getMaterialIndex(const std::string& materialName) const {
		int id = 0;
		for (auto& mtl : mtls) {
			if (mtl.name() == materialName) break;
			id++;
		}
		if (id >= (int)mtls.size()) return 0;
		return id;
	}

	SimpleMaterial* SimpleMaterialLibrary::set_mtl(std::string&& name) {
		SimpleMaterial newmtl;
		newmtl.setName(name);

		// look for existing material
		for (auto& mtl : mtls) {
			if (mtl.name() == name) {
				HFLOGWARN("Duplicate material found '%s'", newmtl.name_cstr());
				return &mtl;
			}
		}

		// add to end
		//TODO: What do we use renderIndex for?
		//newmtl.renderIndex = (int)mtls.size();
		mtls.push_back(newmtl);
		return &mtls.back();
	}

	bool SimpleMaterialLibrary::readMap(const std::string& mapname, std::string& key, const std::string& pathToMap) {
		// Check if we have added this map already
		if (maps.count(key)) {
			return true;
		}

		//if (fpi.notFound()) {
		//	HFLOGWARN("Map %s '%s' cannot be located", mapname.c_str(), pathToMap.c_str());
		//	pathToMap = fpi.filename();
		//	return false;
		//}

		// update the information in the map list
		maps[key] = pathToMap;
		return true;
	}

	std::ostream& SimpleMaterialLibrary::printMap(std::ostream& ostr,
												  const std::string& maptype,
												  const std::string& mapname) const {
		std::string lowerName = tolower(mapname);
		if (!maptype.empty() && maps.count(lowerName)) {
			WriteLabel(ostr, maptype);
			WriteString(ostr, maps.at(lowerName));
		}
		else {
			HFLOGERROR("map '%s' not found", mapname.c_str());
		}
		return ostr;
	}
}
