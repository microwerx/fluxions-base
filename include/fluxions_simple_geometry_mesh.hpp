#ifndef FLUXIONS_SIMPLE_GEOMETRY_MESH_HPP
#define FLUXIONS_SIMPLE_GEOMETRY_MESH_HPP

#include <fluxions_base.hpp>

namespace Fluxions {
	class SimpleGeometryMesh {
	public:
		// These constants match the GL_POINTS, GL_LINES, ... constants
		enum class SurfaceType {
			Points = 0,
			Lines = 1,
			LineLoop = 2,
			LineStrip = 3,
			Triangles = 4,
			TriangleStrip = 5,
			TriangleFan = 6
		};


		struct Vertex {
			Vector3f position;
			Vector3f normal;
			Vector2f texcoord;
			Color4f color;
			Vector3f tangent;
			Vector3f binormal;

			// future, for padding right now
			Vector4ub boneIndex;
			Vector4f boneWeights;
			float sh[9];
		};


		struct Surface {
			SurfaceType mode = SurfaceType::Triangles;
			unsigned first = 0;
			unsigned count = 0;
			std::string materialLibrary;
			std::string materialName;
			std::string surfaceName;
			int materialId = -1;
		};


		SimpleGeometryMesh();
		~SimpleGeometryMesh();


		bool loadOBJ(const std::string& filename);
		bool saveOBJ(const std::string& filename) const;
		int saveOBJByMaterial(const std::string& filename,
							  const std::string& mtllib,
							  const std::string& materialName) const;
		int saveOBJByMaterial(const std::string& filename,
							  const std::string& materialName,
							  int materialId) const;
		bool saveCache(const std::string& filename) const;
		bool loadCache(const std::string& filename);
		void computeTangentVectors();
		void clear();
		void resize(int vertexCount, int indexCount, int surfaceCount = 1);
		void createSimpleModel(int vertexCount, int indexCount, int surfaceCount = 1);
		void transform(const Matrix4f& mat);

		// Drawing commands //////////////////////////////////////////

		// Return the number of indexes
		int getIndexCount() const { return (int)Indices.size(); }

		// Return the number of vertexes
		int getVertexCount() const { return (int)Vertices.size(); }

		// Return a reference to a particular vertex.
		Vertex& getVertex(int i) { return Vertices[i]; }

		// Set the current material
		inline void setMaterial(const std::string& mtl) { currentMaterial_ = mtl; }

		// Set the current material library
		inline void setMaterialLibrary(const std::string& mtllib) { currentMaterialLibrary_ = mtllib; }

		// Start drawing a new surface (aka sub mesh)
		inline void beginSurface(SurfaceType mode) {
			Surface newSurface;
			newSurface.mode = mode;
			newSurface.first = getIndexCount();
			newSurface.count = 0;
			newSurface.materialLibrary = currentMaterialLibrary_;
			newSurface.materialName = currentMaterial_;
			Surfaces.push_back(newSurface);
		}

		// Set the current vertex position
		void position3f(float x, float y, float z, bool addIndex_ = false) {
			curVertexAttrib_.position.reset(x, y, z);
			Vertices.push_back(curVertexAttrib_);
			dirty = true;
			if (addIndex_) {
				addIndex(-1);
			}
		}

		// Set the current vertex position
		void position3f(const Vector3f v, bool addIndex_ = false) {
			curVertexAttrib_.position = v;
			Vertices.push_back(curVertexAttrib_);
			dirty = true;
			if (addIndex_) {
				addIndex(-1);
			}
		}

		// Set the current vertex normal
		void normal3f(float x, float y, float z) {
			curVertexAttrib_.normal.reset(x, y, z);
		}

		// Set the current vertex texture coordinate
		void texcoord2f(float s, float t) {
			curVertexAttrib_.texcoord.reset(s, t);
		}

		// Set the current vertex color
		void color3f(float r, float g, float b) {
			curVertexAttrib_.color.reset(r, g, b, 1.0f);
		}

		// Set the current vertex color
		void color3f(const Color3f color) {
			curVertexAttrib_.color = color;
		}

		// Set the current vertex color
		void color4f(float r, float g, float b, float a) {
			curVertexAttrib_.color.reset(r, g, b, a);
		}

		// Set the current vertex tangent
		void tangent3f(float x, float y, float z) {
			curVertexAttrib_.tangent.reset(x, y, z);
		}

		// Set the current vertex binormal
		void binormal3f(float x, float y, float z) {
			curVertexAttrib_.binormal.reset(x, y, z);
		}

		// Set the current vertex attribute with 4 floats
		inline void attrib4f(int i, float x, float y, float z, float w, bool addIndex_ = false) {
			switch (i) {
			case 0: curVertexAttrib_.position.reset(x, y, z);
				break;
			case 1: curVertexAttrib_.normal.reset(x, y, z);
				break;
			case 2: curVertexAttrib_.texcoord.reset(x, y);
				break;
			case 3: curVertexAttrib_.color.reset(x, y, z, w);
				break;
			case 4: curVertexAttrib_.tangent.reset(x, y, z);
				break;
			case 5: curVertexAttrib_.binormal.reset(x, y, z);
				break;
			default:
				return;
			}
			if (i == 0) {
				Vertices.push_back(curVertexAttrib_);
				dirty = true;
				if (addIndex_) {
					addIndex(-1);
				}
			}
		}

		// Add an index for the current surface
		inline void addIndex(int i = -1) {
			if (i < 0)
				Indices.push_back(getIndexCount());
			else if (within(i, 0, getVertexCount() - 1))
				Indices.push_back(i);
			else
				return;
			if (!Surfaces.empty())
				Surfaces.back().count++;
			dirty = true;
		}

		// Set a 1 component vertex attribute with 1 float
		inline void attrib1f(int i, float x, bool addIndex = false) { attrib4f(i, x, 0.0f, 0.0f, 1.0f, addIndex); }

		// Set a 2 component vertex attribute with 2 floats
		inline void attrib2f(int i, float x, float y, bool addIndex = false) { attrib4f(i, x, y, 0.0f, 1.0f, addIndex); }

		// Set a 2 component vertex attribute with a Vector2f
		inline void attrib2f(int i, const Vector2f& v, bool addIndex = false) { attrib4f(i, v.x, v.y, 0.0f, 1.0f, addIndex); }

		// Set a 3 component vertex attribute with 3 floats
		inline void attrib3f(int i, float x, float y, float z, bool addIndex = false) { attrib4f(i, x, y, z, 1.0f, addIndex); }

		// Set a 3 component vertex attribute with a Vector3f
		inline void attrib3f(int i, const Vector3f& v, bool addIndex = false) { attrib4f(i, v.x, v.y, v.z, 1.0f, addIndex); }

		// Set a 3 component vertex attribute with a Color3f
		inline void attrib3f(int i, const Color3f& v, bool addIndex = false) { attrib4f(i, v.r, v.g, v.b, 1.0f, addIndex); }

		// Set a 4 component vertex attribute with a Vector 4f
		inline void attrib4f(int i, const Vector4f& v, bool addIndex = false) { attrib4f(i, v.x, v.y, v.z, v.w, addIndex); }

		// Set a 4 component vertex attribute with a Color4f
		inline void attrib4f(int i, const Color4f& v, bool addIndex = false) { attrib4f(i, v.r, v.g, v.b, v.a, addIndex); }

		// Memory Buffer Helpers /////////////////////////////////////

		// Return pointer to the vertex data
		inline const void* getVertexData() const { return &Vertices[0]; }

		// Return the size of the vertex data
		inline size_t getVertexDataSize() const { return (size_t)(sizeof(Vertex) * Vertices.size()); }

		// Return the pointer to the index data
		inline const void* getIndexData() const { return &Indices[0]; }

		// Return the size of the index data
		inline size_t getIndexDataSize() const { return (size_t)(sizeof(unsigned) * Indices.size()); }
		
		// Vertex Array Helpers //////////////////////////////////////

		// Return if an attribute is enabled
		bool isAttribEnabled(int i) const { return (i >= 0 && i <= 3); }

		// Return the name of the attribute name
		const char* getAttribName(int i) const {
			switch (i) {
			case 0: return "aPosition";
			case 1: return "aNormal";
			case 2: return "aTexCoord";
			case 3: return "aColor";
			}
			return nullptr;
		}

		// Return whether an attribute should be normalized
		bool isAttribNormalized(int i) const { if (i < 0) return true; return false; }
		
		// Return the offset into the vertex data
		int getVertexOffset(int i) const {
			switch (i) {
			case 0: return 0;
			case 1: return 3 * sizeof(float);
			case 2: return 6 * sizeof(float);
			case 3: return 8 * sizeof(float);
			}
			return 0;
		}

		// Properties ////////////////////////////////////////////////

		// The name of the geometry mesh
		std::string name;
		// The path where the geometry mesh is located
		std::string path;
		// The map of material libraries (first=identifier, last=original)
		std::map<std::string, std::string> mtllibs;
		// The map of materials (first=identifier, last=original)
		std::map<std::string, std::string> Materials;
		// The array of vertexes
		std::vector<Vertex> Vertices;
		// The array of indexes
		std::vector<unsigned> Indices;
		// The array of surfaces
		std::vector<Surface> Surfaces;
		// The bounding box of the entire object
		BoundingBoxf BoundingBox;

	private:
		Vertex curVertexAttrib_;
		std::string currentMaterial_;
		std::string currentMaterialLibrary_;
		bool dirty{ true };
		// note: modifies mtllibname
		bool add_mtllib(std::istream& istr, std::string& mtllibname, const std::string& basepath);
	};
}

#endif
