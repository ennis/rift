#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <renderer2.hpp>
#include <unordered_map>
#include <array>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <boost/filesystem/path.hpp>

class Shader; 

// TODO remove this
std::string loadShaderSource(const char *path);

//=============================================================================
//
// classe Effect
//
// C'est une petite (?) couche d'abstraction sur les shaders. Elle gère la création
// des shaders, fait du preprocessing (gère les includes et les #defines)
//
// Un Effect peut compiler plusieurs shaders différents en fonction des 'keywords'
// passés en entrée: le résultat de la compilation est bien sûr mis en cache
// pour ne pas avoir à les recompiler à chaque frame.
// En combinant cela avec des includes, cela permet de générer automatiquement 
// des shaders pour différents paramètres de scène: fog, ombres, types de lumière, etc.
// Les shaders compilés sont détruits au même moment que l'effect
//
// Exemple:
// Chargement d'un Effect:
//
//		Effect e ("resources/shaders/test.glsl");	// combined source
//
// Compilation d'un shader à partir d'un effect:
//
// 		Effect::Keyword keywords[] = 
//			{ 
//				{ "DIRECTIONAL_LIGHT", "" } 	// signale au shader que les lumières sont directionnelles
//				{ "NO_SHADOW_MAP", "" }			// pas de shadow mapping
//			};
//		Shader *shader = e.compileShader(renderer, std::make_array_ref(keywords));
// 
class Effect
{
public:
	struct Keyword
	{
		std::string define;
		std::string value;
	};

	// nullable
	Effect() = default;
	// TODO change initializer_list to array_ref
	Effect(
		const char *combinedSourcePath, 
		std::initializer_list<Effect::Keyword> keywords = {})
	{
		loadFromFile(combinedSourcePath, keywords);
	}

	// TODO change initializer_list to array_ref
	Effect(
		const char *vsPath, 
		const char *fsPath, 
		std::initializer_list<Effect::Keyword> keywords = {})
	{
		loadFromFile(vsPath, fsPath, keywords);
	}

	// noncopyable
	Effect(const Effect &) = delete;
	Effect &operator=(const Effect &) = delete;
	
	// moveable
	Effect(Effect &&rhs)
	{
		*this = std::move(rhs);
	}

	Effect &operator=(Effect &&rhs)
	{
		mID = rhs.mID;
		mVSPath = std::move(rhs.mVSPath); 
		mFSPath = std::move(rhs.mFSPath);
		mVertexShader = std::move(rhs.mVertexShader);
		mFragmentShader = std::move(rhs.mFragmentShader);
		mCombinedSource = rhs.mCombinedSource;
		mKeywords = std::move(mKeywords);
		mShaderCache = std::move(rhs.mShaderCache);
		rhs.mID = 0;
		return *this;
	}

	~Effect()
	{//TODO
	}

	// Compile a shader with the given list of keywords
	// TODO once the new effect system lands, there will be no need for preprocessing:
	// All the possible permutations will be created before runtime
	Shader *compileShader(
		Renderer &renderer, 
		std::array_ref<Keyword> additionalKeywords
		);

	// TODO load API-agnostic effect files

	bool isNull() const {
		return mID == 0;
	}

private:
	void loadFromFile(
		const char *combinedSourcePath,
		std::initializer_list<Effect::Keyword> keywords = {}
		);

	void loadFromFile(
		const char *vsPath,
		const char *fsPath,
		std::initializer_list<Effect::Keyword> keywords = {}
		);

	static unsigned int sCurrentID;
	unsigned int mID = 0; 
	boost::filesystem::path mVSPath;
	boost::filesystem::path mFSPath;
	std::string mVertexShader;
	std::string mFragmentShader;
	// true if mVertexShader contains both the vertex shader and the fragment shader
	bool mCombinedSource;
	// list of #defines
	std::vector<Keyword> mKeywords;
	// Shader cache
	std::unordered_map<uint64_t, std::unique_ptr<Shader> > mShaderCache;
};


#endif
