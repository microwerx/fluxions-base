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
		c3fImages_.clear();
		c4fImages_.clear();
		c3ubImages_.clear();
		c4ubImages_.clear();
		SimpleLoadableResource::unload();
	}


	void SimpleMapLibrary::cacheToDisk() {
		for (auto& i : c3fImages_) {
			i.second.reset();
		}
		for (auto& i : c4fImages_) {
			i.second.reset();
		}
		for (auto& i : c3ubImages_) {
			i.second.reset();
		}
		for (auto& i : c4ubImages_) {
			i.second.reset();
		}
		SimpleLoadableResource::cacheToDisk();
	}


	void SimpleMapLibrary::fetchCache() {
		for (auto& i : c3fImages_) {
			if (i.second.pixels.empty())
				LoadImage3f(i.first, i.second);
		}
		for (auto& i : c4fImages_) {
			if (i.second.pixels.empty())
				LoadImage4f(i.first, i.second);
		}
		for (auto& i : c3ubImages_) {
			if (i.second.pixels.empty())
				LoadImage3ub(i.first, i.second);
		}
		for (auto& i : c4ubImages_) {
			if (i.second.pixels.empty())
				LoadImage4ub(i.first, i.second);
		}
		SimpleLoadableResource::fetchCache();
	}


	size_t SimpleMapLibrary::sizeInBytes() const {
		size_t size = 0;
		for (auto& i : c3fImages_)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c4fImages_)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c3ubImages_)
			size += i.first.size() + i.second.sizeInBytes();
		for (auto& i : c4ubImages_)
			size += i.first.size() + i.second.sizeInBytes();
		return size;
	}


	bool SimpleMapLibrary::addMap(const std::string& path, const FilePathFinder& fpf) {
		std::string shortestPath = fpf.findShortestPath(path);
		if (shortestPath.empty()) {
			HFLOGERROR("'%s' was not found", path.c_str());
			return false;
		}
		const FilePathInfo& fpi = fpf.fpi();
		std::string ext = fpi.extension();
		tolower(ext);
		const std::string name = fpi.shortestPath();
		bool result{ false };
		if (ext == ".exr") {
			auto& image = c4fImages_[name];
			result = LoadImage4f(name, image);
		}
		else if (ext == ".pfm") {
			auto& image = c3fImages_[name];
			result = LoadImage3f(name, image);
		}
		else if (ext == ".png") {
			auto& image = c4ubImages_[name];
			result = LoadImage4ub(name, image);
		}
		else if (ext == ".jpg" || ext == ".ppm") {
			auto& image = c3ubImages_[name];
			result = LoadImage3ub(name, image);
		}
		else {
			HFLOGERROR("'%s' is not a supported map", path.c_str());
		}
		if (!result) {
			HFLOGERROR("'%s' had an error loading", path.c_str());
		}
		else {
			HFLOGINFO("Map '%s' loaded", path.c_str());
		}
		return result;
	}
}
