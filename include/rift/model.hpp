#ifndef MODEL_HPP
#define MODEL_HPP

#include <renderer.hpp>
#include <mesh.hpp>

namespace ModelLoadHint
{
	static const unsigned int Unpack = (1 << 0);
	static const unsigned int Static = (1 << 1);
};

/**
 * @brief Classe représentant un modèle 3D
 * @details 
 * 
 */
class Model
{
public:
	/**
	 * @brief XXX
	 * @details non utilisé
	 */
	struct GPUVertex {
		// packed layout (less precise but more compact)
		glm::vec3 position;
		glm::uint32 normal; // packSnorm3x10_1x2
		glm::uint32 tangent; // packSnorm3x10_1x2
		glm::uint32 bitangent;	// packSnorm3x10_1x2
		glm::uint32 texcoord;	// packUnorm2x16
	}; 
	
	/**
	 * @brief XXX
	 * @details non utilisé
	 * 
	 */
	struct GPUSkinnedVertex {
		glm::vec3 position;
		glm::uint32 normal; // packSnorm3x10_1x2
		glm::uint32 tangent; // packSnorm3x10_1x2
		glm::uint32 bitangent;	// packSnorm3x10_1x2
		glm::uint32 texcoord;	// packUnorm2x16
		glm::u8vec4 boneIds;
		glm::uint64 boneWeights; // packUnorm4x16
	};

	/**
	 * @brief Un os du squelette
	 * @details 
	 * 
	 */
	struct Bone {
		// Index (dans la table des os) de l'os parent dans le squelette
		int parent;
		// Transformation (du repère local du parent vers le repère local de cet os)
		glm::mat4 transform;
		// Inverse bind pose transformation
		// c'est peut-être utilisé quelque part mais je sais plus où
		glm::mat4 invBindPose;
		// ça à l'air d'être utilisé nulle part
		glm::mat4 finalTransform;
		// Indices des os descendants
		std::vector<int> children;
	};

	/**
	 * @brief XXX
	 * @details inutilisé
	 * 
	 */
	struct GPUBone {
		glm::mat4 transform;
	};

	/**
	 * @brief Sous-mesh
	 * @details Représente une partie des données du maillage
	 * 
	 */
	struct Submesh {
		// Index du premier vertex 
		unsigned int startVertex;
		// Index du premier index (dans la table des indices)
		unsigned int startIndex;
		// Nombre de vertices dans la sous-mesh
		unsigned int numVertices;
		// Nombre d'indices
		unsigned int numIndices;
		// Os du squelette attaché à cette sous-mesh
		int bone;
	};

	/**
	 * @brief Constructeur
	 * @details Constructeur
	 * 
	 * @param renderer Réference vers le renderer
	 * 
	 * TODO Instance globale de Renderer
	 */
	Model(Renderer &renderer);

	/**
	 * @brief Constructeur
	 * @details Constructeur; appelle également loadFromFile
	 * 
	 * @param renderer Réference vers le renderer
	 * @param filePath Chemin du fichier à charger
	 * 
	 * TODO Instance globale de Renderer
	 */
	Model(Renderer &renderer, const char *filePath);
	~Model();
	
	/**
	 * @brief Chargement depuis un fichier
	 * @details 
	 * 
	 * @param filePath Chemin du fichier à charger
	 * @param hints Inutilisé
	 */
	void loadFromFile(const char *filePath, unsigned int hints = 0);

	unsigned int numVertices() const {
		return mNumVertices;
	}
	unsigned int numIndices() const {
		return mNumIndices;
	}
	bool isSkinned() const {
		return !mBoneIDs.empty();
	}
	std::vector<Bone> const &getBones() const {
		return mBones;
	}
	std::vector<glm::vec3> const &getPositions() const {
		return mPositions;
	}
	std::vector<glm::vec3> const &getNormals() const {
		return mNormals;
	}
	std::vector<glm::vec3> const &getTangents() const {
		return mTangents;
	}
	std::vector<glm::vec3> const &getBitangents() const {
		return mBitangents;
	}
	std::vector<glm::vec2> const &getTexcoords(unsigned int id = 0) const {
		return mTexcoords;
	}
	std::vector<glm::u8vec4> const &getBoneIndices() const {
		return mBoneIDs;
	}
	std::vector<glm::vec4> const &getBoneWeights() const {
		return mBoneWeights;
	}
	std::vector<uint16_t> const &getIndices() const {
		return mIndices;
	}
	std::vector<Model::Submesh> const &getSubmeshes() const {
		return mSubmeshes;
	}
	
private:
	Renderer &mRenderer;
	std::vector<Submesh> mSubmeshes;
	//std::vector<Model::Vertex> mVertices;
	std::vector<uint16_t> mIndices;
	std::vector<Bone> mBones;
	std::unique_ptr<Mesh> mMesh;
	unsigned int mNumVertices;
	unsigned int mNumIndices;
	std::vector<glm::vec3> mPositions;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec3> mTangents;
	std::vector<glm::vec3> mBitangents;
	std::vector<glm::vec2> mTexcoords;
	std::vector<glm::u8vec4> mBoneIDs;
	std::vector<glm::vec4> mBoneWeights;
};

 
#endif /* end of include guard: MODEL_HPP */