#include <effect.hpp>
#include <sstream>
#include <algorithm>
#include <regex>
#include <vector>
#include <log.hpp>

unsigned int Effect::sCurrentID = 0;

CompiledShader *Effect::compileShader(
	int numAdditionalKeywords, 
	Keyword *additionalKeywords)
{
	LOG << "Hash " << getHash(numAdditionalKeywords, additionalKeywords);
	return nullptr;
}

std::size_t Effect::getHash(int numAdditionalKeywords, Effect::Keyword *additionalKeywords)
{
	std::size_t a = 0;
	std::hash<std::string> hashfn;
	for (int i =0; i < numAdditionalKeywords; ++i) {
		a += hashfn(additionalKeywords[i].define);
		a += hashfn(additionalKeywords[i].value);
	}
	for (auto &k : mKeywords) {
		a += hashfn(k.define);
		a += hashfn(k.value);
	}
	return a;
}
