#ifndef FLUXIONS_SIMPLE_LOADABLE_RESOURCE_HPP
#define FLUXIONS_SIMPLE_LOADABLE_RESOURCE_HPP

#include <string>

namespace Fluxions {
	class SimpleLoadableResource {
	public:
		SimpleLoadableResource() {}
		virtual ~SimpleLoadableResource() {}

		// id() returns an identifier for this object (e.g. UUID or lowerCase)
		const std::string& id() const { return id_; }

		// setId() sets an identifier for this object (e.g. UUID or lowerCase)
		void setId(const std::string& identifier) { id_ = identifier; }

		// name() returns the human readable name of the object
		const std::string& name() const { return name_; }

		// setName() sets the human readable name of the object
		void setName(const std::string& nameOfObject) { name_ = nameOfObject; }

		// path() returns the file path to the object for loading
		const std::string& path() const { return path_; }

		// setPath(path) sets the file path for this object for loading
		void setPath(const std::string& pathToObject) { path_ = pathToObject; }

		// loaded() returns true if the object was loaded
		bool loaded() const { return loaded_; }

		// cached() returns true if the object has been cached to disk
		bool cached() const { return cached_; }

		// load() causes the object to be loaded from disk
		virtual bool load() = 0;

		// unload() causes the object to be unloaded from memory
		virtual bool unload() = 0;

		// cacheToDisk() causes the object to be temporarily saved to disk and unloaded from memory
		virtual bool cacheToDisk() = 0;

		// fetchCache() causes the object to be loaded from disk and loaded to memory
		virtual bool fetchCache() = 0;

		// status() creates a human readable string about the status of this object
		inline std::string status() const {
			std::string state = "'" + name() + "' is ";
			state += loaded() ? "loaded" : "not loaded";
			state += " and ";
			state += cached() ? "cached" : "not cached";
			return state;
		}

	private:
		std::string id_{};		// a UUID or lowerCase label for this object
		std::string name_{};	// the human readable name of the object
		std::string path_{};	// the path to the object
		bool loaded_{ false };	// true if the object was loaded
		bool cached_{ false };	// true if the object is not in memory
	};
} // namespace Fluxions

#endif
