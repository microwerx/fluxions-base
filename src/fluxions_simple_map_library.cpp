#include "fluxions_base_pch.hpp"
#include <fluxions_file_path_info.hpp>
#include <fluxions_simple_map_library.hpp>


namespace Fluxions {
	SimpleMapLibrary::SimpleMapLibrary() {

	}


	SimpleMapLibrary::~SimpleMapLibrary() {

	}


	bool SimpleMapLibrary::load() {
		// We load up all the images
		fetchCache();
		return SimpleLoadableResource::load();
	}


	void SimpleMapLibrary::unload() {
		cacheToDisk();
		c3fImages.clear();
		c4fImages.clear();
		c3ubImages.clear();
		c4ubImages.clear();
		SimpleLoadableResource::unload();
	}


	void SimpleMapLibrary::cacheToDisk() {
		for (auto& i : c3fImages) {
			i.second.reset();
		}
		for (auto& i : c4fImages) {
			i.second.reset();
		}
		for (auto& i : c3ubImages) {
			i.second.reset();
		}
		for (auto& i : c4ubImages) {
			i.second.reset();
		}
		SimpleLoadableResource::cacheToDisk();
	}


	void SimpleMapLibrary::fetchCache() {
		for (auto& i : c3fImages) {
			if (i.second.pixels.empty())
				LoadImage3f(i.first, i.second);
		}
		for (auto& i : c4fImages) {
			if (i.second.pixels.empty())
				LoadImage4f(i.first, i.second);
		}
		for (auto& i : c3ubImages) {
			if (i.second.pixels.empty())
				LoadImage3ub(i.first, i.second);
		}
		for (auto& i : c4ubImages) {
			if (i.second.pixels.empty())
				LoadImage4ub(i.first, i.second);
		}
		SimpleLoadableResource::fetchCache();
	}


	size_t SimpleMapLibrary::sizeInBytes() const {
		size_t size = 0;
		for (auto& i : c3fImages)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c4fImages)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c3ubImages)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c4ubImages)
			size += i.first.size() + i.second.sizeInBytes();
		return size;
	}


	bool SimpleMapLibrary::addMap(const std::string& path) {
		FilePathInfo fpi(path);
		if (fpi.notFound()) {
			HFLOGERROR("'%s' was not found", path.c_str());
			return false;
		}
		std::string ext = fpi.extension();
		tolower(ext);
		const std::string name = fpi.shortestPath();
		bool result{ false };
		if (ext == ".exr") {
			auto& image = c4fImages[name];
			result = LoadImage4f(name, image);
		}
		else if (ext == ".pfm") {
			auto& image = c3fImages[name];
			result = LoadImage3f(name, image);
		}
		else if (ext == ".png") {
			auto& image = c4ubImages[name];
			result = LoadImage4ub(name, image);
		}
		else if (ext == ".jpg" || ext == ".ppm") {
			auto& image = c3ubImages[name];
			result = LoadImage3ub(name, image);
		}
		else {
			HFLOGERROR("'%s' is not a supported map", path.c_str());
		}
		if (!result) {
			HFLOGERROR("'%s' had an error loading", path.c_str());
		}
		return result;
	}
}
